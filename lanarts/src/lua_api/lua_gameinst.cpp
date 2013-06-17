/*
 * lua_gameinst.cpp:
 *  Representation of a GameInst in Lua.
 */

#include <lua.hpp>

#include <lua_api/lua_newapi.h>

#include <luawrap/luawrap.h>
#include <luawrap/members.h>

#include <luawrap/macros.h>

#include "data/lua_game_data.h"

#include "gamestate/GameState.h"

#include "data/game_data.h"

#include "objects/GameInst.h"
#include "objects/enemy/EnemyInst.h"
#include "objects/FeatureInst.h"
#include "objects/ItemInst.h"
#include "objects/player/PlayerInst.h"
#include "objects/CombatGameInst.h"

#include "lua_api.h"
#include "lua_gameinst.h"

#include "draw/SpriteEntry.h"

static int lapi_gameinst_stats(lua_State* L) {
	lua_push_combatstats(L, luawrap::get<GameInst*>(L, 1));
	return 1;
}

/* Use a per-instance lua table as a fallback */
static int lapi_gameinst_getter_fallback(lua_State* L) {
	LuaValue& variables = luawrap::get<GameInst*>(L, 1)->lua_variables;
	if (variables.empty()) {
		lua_pushnil(L);
	} else {
		variables.push();
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
	}
	return 1;
}

/* Use a per-instance lua table as a fallback */
static int lapi_gameinst_setter_fallback(lua_State* L) {
	LuaValue& variables = luawrap::get<GameInst*>(L, 1)->lua_variables;
	if (variables.empty()) {
		variables.init(L);
		variables.newtable();
	}
	variables.push();
	lua_pushvalue(L, 2);
	lua_pushvalue(L, 3);
	lua_settable(L, -3);
	return 1;
}

static LuaValue lua_gameinst_base_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(GameInst*);

	LuaValue meta = luameta_new(L, "Font");
	meta["__isgameinst"] = true;

	LuaValue methods = luameta_constants(meta);
	LUAWRAP_METHOD(methods, step, OBJ->step(lua_api::gamestate(L)));
	LUAWRAP_METHOD(methods, draw, OBJ->draw(lua_api::gamestate(L)));

	LuaValue getters = luameta_getters(meta);
	LUAWRAP_GETTER(getters, xy, OBJ->pos());
	luawrap::bind_getter(getters["x"], &GameInst::x);
	luawrap::bind_getter(getters["y"], &GameInst::y);
	luawrap::bind_getter(getters["id"], &GameInst::y);
	luawrap::bind_getter(getters["depth"], &GameInst::depth);
	luawrap::bind_getter(getters["radius"], &GameInst::radius);
	luawrap::bind_getter(getters["target_radius"], &GameInst::target_radius);
	luawrap::bind_getter(getters["floor"], &GameInst::current_floor);

	getters["stats"].bind_function(lapi_gameinst_stats);

	// Use table as fallback for getting values
	lua_pushcfunction(L, &lapi_gameinst_getter_fallback);
	luameta_defaultgetter(meta, LuaStackValue(L, -1));
	lua_pop(L, 1);

	LuaValue setters = luameta_setters(meta);
	LUAWRAP_SETTER(setters, x, double, OBJ->update_position(VAL, OBJ->y));
	LUAWRAP_SETTER(setters, y, double, OBJ->update_position(OBJ->x, VAL));
	LUAWRAP_SETTER(setters, xy, PosF, OBJ->update_position(VAL.x, VAL.y));

	// Use table as fallback for setting values
	lua_pushcfunction(L, &lapi_gameinst_setter_fallback);
	luameta_defaultsetter(meta, LuaStackValue(L, -1));
	lua_pop(L, 1);

	return meta;
}

static int lapi_combatgameinst_damage(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	EffectiveAttackStats attack;
	int nargs = lua_gettop(L);
	attack.damage = round(lua_tointeger(L, 2));
	attack.power = round(lua_tointeger(L, 3));
	attack.magic_percentage = nargs >= 4 ? lua_tonumber(L, 4) : 1.0f;
	attack.resist_modifier = nargs >= 5 ? lua_tonumber(L, 5) : 1.0f;

	luawrap::get<CombatGameInst*>(L, 1)->damage(lua_api::gamestate(L), attack);
	return 0;
}

static void lapi_combatgameinst_heal_hp(CombatGameInst* inst, int hp) {
	CoreStats& core = inst->core_stats();
	CoreStats& ecore = inst->effective_stats().core;
	core.heal_hp(hp, ecore.max_hp);
	ecore.hp = core.hp;
}

static void lapi_combatgameinst_heal_mp(CombatGameInst* inst, int mp) {
	CoreStats& core = inst->core_stats();
	CoreStats& ecore = inst->effective_stats().core;
	core.heal_mp(mp, ecore.max_mp);
	ecore.mp = core.mp;
}

static int lapi_do_nothing(lua_State *L) {
    return 0;
}

static LuaValue lua_combatgameinst_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(CombatGameInst*);

	LuaValue meta = lua_gameinst_base_metatable(L);

	LuaValue getters = luameta_getters(meta);
	luawrap::bind_getter(getters["vx"], &CombatGameInst::vx);
	luawrap::bind_getter(getters["vy"], &CombatGameInst::vy);

	LuaValue methods = luameta_constants(meta);
	LUAWRAP_METHOD(methods, heal_fully, OBJ->stats().core.heal_fully());
	LUAWRAP_METHOD(methods, direct_damage, OBJ->damage(lua_api::gamestate(L), lua_tointeger(L, 2)));
	LUAWRAP_METHOD(methods, melee, luawrap::push(L, OBJ->melee_attack(lua_api::gamestate(L), luawrap::get<CombatGameInst*>(L, 2), OBJ->equipment().weapon(), true)) );

	methods["damage"].bind_function(lapi_combatgameinst_damage);
	methods["heal_hp"].bind_function(lapi_combatgameinst_heal_hp);
	methods["heal_mp"].bind_function(lapi_combatgameinst_heal_mp);

	LUAWRAP_METHOD(methods, add_effect, OBJ->effects().add(lua_api::gamestate(L), OBJ, effect_from_lua(L, 2), lua_tointeger(L, 3)).push() );
	LUAWRAP_GETTER(methods, has_effect, OBJ->effects().get(effect_from_lua(L, 2)) != NULL);

	methods["reset_rest_cooldown"].bind_function(lapi_do_nothing);

	return meta;
}

static LuaValue lua_enemyinst_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(EnemyInst*);

	LuaValue meta = lua_combatgameinst_metatable(L);
	LuaValue methods = luameta_constants(meta), getters = luameta_getters(meta), setters = luameta_setters(meta);

	LUAWRAP_GETTER(getters, name, OBJ->etype().name);
	LUAWRAP_GETTER(getters, unique, OBJ->etype().unique);
	LUAWRAP_GETTER(getters, kills, 0);
	LUAWRAP_GETTER(methods, is_local_player, false);

	return meta;
}

static LuaValue lua_playerinst_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(PlayerInst*);
	LuaValue meta = lua_combatgameinst_metatable(L);

	LuaValue getters = luameta_getters(meta);
	LUAWRAP_GETTER(getters, name, OBJ->player_entry(lua_api::gamestate(L)).player_name);
	LUAWRAP_GETTER(getters, class_name, OBJ->class_stats().class_entry().name);
	LUAWRAP_GETTER(getters, kills, OBJ->score_stats().kills);
	LUAWRAP_GETTER(getters, deepest_floor, OBJ->score_stats().deepest_floor);
	LUAWRAP_GETTER(getters, deaths, OBJ->score_stats().deaths);
	LUAWRAP_GETTER(getters, spells, OBJ->stats().spells.spell_id_list());

	LuaValue methods = luameta_constants(meta);
	LUAWRAP_GETTER(methods, is_local_player, OBJ->is_local_player());
	LUAWRAP_METHOD(methods, reset_rest_cooldown, OBJ->cooldowns().reset_rest_cooldown(REST_COOLDOWN));

	return meta;
}

static void lua_gameinst_push_metatable(lua_State* L, GameInst* inst) {
	if (dynamic_cast<PlayerInst*>(inst)) {
		luameta_push(L, &lua_playerinst_metatable);
	} else if (dynamic_cast<EnemyInst*>(inst)) {
		luameta_push(L, &lua_enemyinst_metatable);
	} else if (dynamic_cast<CombatGameInst*>(inst)) {
		luameta_push(L, &lua_combatgameinst_metatable);
	} else {
		luameta_push(L, &lua_gameinst_base_metatable);
	}
}

// Functions required for luawrap
namespace GameInstWrap {
	GameInst* get(lua_State* L, int idx) {
		return *(GameInst**) lua_touserdata(L, idx);
	}

	bool check(lua_State* L, int idx) {
		if (!lua_isuserdata(L, idx)) {
			return false;
		}
		lua_getmetatable(L, idx);
		if (lua_isnil(L, -1)) {
			return false;
		}
		lua_getfield(L, -1, "__isgameinst");
		bool isgameinst = !lua_isnil(L, -1);
		lua_pop(L, 2);
		return isgameinst;
	}
	void push(lua_State* L, GameInst* inst) {
		GameInst** lua_inst = (GameInst**) lua_newuserdata(L, sizeof(GameInst*));
		*lua_inst = inst;
		lua_gameinst_push_metatable(L, inst);
		lua_setmetatable(L, -2);
	}
}

/* TODO: Deprecated */
void lua_gameinst_callback(lua_State* L, LuaValue& value, GameInst* inst) {
	if (value.empty())
		return;
	value.push();
	luawrap::push(L, inst);
	lua_call(L, 1, 0);
}

static GameInst* lua_enemyinst_create(LuaStackValue args) {
	using namespace luawrap;
	GameState* gs = lua_api::gamestate(args);
	int teamid = defaulted(args["team_id"], gs->teams().default_enemy_team());
	const char* enemy_name = args["enemy"].to_str();

	Pos xy = args["xy"].as<Pos>();
	return new EnemyInst(get_enemy_by_name(enemy_name),xy.x, xy.y, teamid);
}

static GameInst* lua_featureinst_create(LuaStackValue args) {
	using namespace luawrap;

	return new FeatureInst(args["xy"].as<Pos>(), (FeatureInst::feature_t)args["type"].to_int(),
			defaulted(args["solid"], false),
			res::sprite_id(args["sprite"].to_str()));
}


static Item lua_item_get(LuaStackValue item) {
	item_id type = get_item_by_name(item["type"].to_str());
	int amount = luawrap::defaulted(item["amount"], 1);
	return Item(type, amount);
}
static GameInst* lua_iteminst_create(LuaStackValue args) {
	using namespace luawrap;
	return new ItemInst(lua_item_get(args), args["xy"].as<Pos>());
}

void lua_register_gameinst(lua_State* L) {
	luawrap::install_type<GameInst*, GameInstWrap::push, GameInstWrap::get, GameInstWrap::check>();
	luawrap::install_dynamic_casted_type<CombatGameInst*, GameInst*>();
	luawrap::install_dynamic_casted_type<PlayerInst*, GameInst*>();
	luawrap::install_dynamic_casted_type<EnemyInst*, GameInst*>();
	luawrap::install_dynamic_casted_type<FeatureInst*, GameInst*>();

	LuaValue globals = luawrap::globals(L);
	LuaValue game_object_table = luawrap::ensure_table(globals["GameObject"]);
	game_object_table["enemy_create"].bind_function(lua_enemyinst_create);
	game_object_table["feature_create"].bind_function(lua_featureinst_create);
	game_object_table["item_create"].bind_function(lua_iteminst_create);

	game_object_table["DOOR_OPEN"] = (int)FeatureInst::DOOR_OPEN;
	game_object_table["DOOR_CLOSED"] = (int)FeatureInst::DOOR_CLOSED;
	game_object_table["DECORATION"] = (int)FeatureInst::DECORATION;
}
