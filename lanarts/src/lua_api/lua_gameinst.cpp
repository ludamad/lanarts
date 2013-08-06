/*
 * lua_gameinst.cpp:
 *  Representation of a GameInst in Lua.
 */

#include <lua.hpp>
#include <stdexcept>

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
#include "objects/store/StoreInst.h"
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

static int lapi_gameinst_freeref(lua_State* L) {
	GameInst::free_reference(luawrap::get<GameInst*>(L, 1));
	return 0;
}

static LuaValue lua_gameinst_base_metatable(lua_State* L) {
	LUAWRAP_SET_TYPE(GameInst*);

	LuaValue meta = luameta_new(L, "GameObject");
	meta["__isgameinst"] = true;

	LuaValue methods = luameta_constants(meta);
	LUAWRAP_METHOD(methods, step, OBJ->step(lua_api::gamestate(L)));
	LUAWRAP_METHOD(methods, draw, OBJ->draw(lua_api::gamestate(L)));
	LUAWRAP_METHOD(methods, init,
			GameState* gs = lua_api::gamestate(L);
			level_id map = lua_gettop(L) <= 0 ? gs->game_world().get_current_level_id() : lua_tointeger(L, 1);
			gs->add_instance(map, OBJ);
	);

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

	luameta_gc(meta, lapi_gameinst_freeref);

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
		GameInst::retain_reference(inst);
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

static void set_gameinst_member(GameInst* inst, LuaStackValue args, const char* member) {
	lua_State* L = args.luastate();
	args[member].push();
	if (!lua_isnil(L, -1)) {
		LuaValue& v = inst->lua_variables;
		if (v.empty()) {
			v.init(L);
			v.newtable();
		}
		v[member].pop();
	} else {
		lua_pop(L, 1);
	}
}

static GameInst* do_instance_init(GameState* gs, GameInst* inst, LuaStackValue args) {
	set_gameinst_member(inst, args, "on_init");
	set_gameinst_member(inst, args, "on_deinit");
	set_gameinst_member(inst, args, "on_step");
	set_gameinst_member(inst, args, "on_draw");
	set_gameinst_member(inst, args, "on_player_interact");
	if (!args["radius"].isnil()) {
		inst->target_radius = args["radius"].to_int();
		inst->radius = std::min(15, inst->target_radius);
	}
	if (args["do_init"].isnil() || args["do_init"].to_bool()) {
		level_id id;
		if (args["map"].isnil()) {
			if (!gs->get_level()) {
				throw std::runtime_error("Attempt to initialize object with no active levels! Try using with do_init = false");
			}
			id = gs->game_world().get_current_level_id();
		} else {
			id = args["map"].to_int();
		}
		gs->add_instance(id, inst);
	}
	return inst;
}

static GameInst* enemy_create(LuaStackValue args) {
	GameState* gs = lua_api::gamestate(args);
	int etype = get_enemy_by_name(args["type"].to_str());
	Pos xy = args["xy"].as<Pos>();
	EnemyInst* inst = new EnemyInst(etype, xy.x, xy.y, gs->teams().default_enemy_team());
	return do_instance_init(gs, inst, args);
}

static GameInst* feature_create(LuaStackValue args) {
	using namespace luawrap;
	GameState* gs = lua_api::gamestate(args);

	FeatureInst* inst = new FeatureInst(args["xy"].as<Pos>(), (FeatureInst::feature_t)args["type"].to_int(),
			defaulted(args["solid"], false),
			res::sprite_id(args["sprite"].to_str()),
			defaulted(args["depth"], (int)FeatureInst::DEPTH),
			defaulted(args["frame"], 0));
	return do_instance_init(gs, inst, args);
}

static Item lua_item_get(LuaStackValue item) {
	item_id type = get_item_by_name(item["type"].to_str());
	int amount = luawrap::defaulted(item["amount"], 1);
	return Item(type, amount);
}

static GameInst* store_create(LuaStackValue args) {
	using namespace luawrap;
	lua_State* L = args.luastate();
	GameState* gs = lua_api::gamestate(args);

	// Determine inventory
	StoreInventory inventory;
	LuaField items = args["items"];
	int size = items.objlen();
	for (int i = 1; i <= size; i++) {
		items[i].push();
		LuaStackValue item(L, -1);
		item_id type = get_item_by_name(item["type"].to_str());
		ItemEntry& entry = get_item_entry(type);
		int amount = luawrap::defaulted(item["amount"], 1);
		int cost = gs->rng().rand(entry.shop_cost.multiply(amount));
		inventory.add(Item(type, amount), cost);
	}
	sprite_id spr_id = res::sprite_id(args["sprite"].to_str());

	StoreInst* inst = new StoreInst(args["xy"].as<Pos>(),
			defaulted(args["solid"], false),
			spr_id, inventory,
			defaulted(args["frame"], 0));
	return do_instance_init(gs, inst, args);
}

static GameInst* item_create(LuaStackValue args) {
	using namespace luawrap;
	GameState* gs = lua_api::gamestate(args);
	return do_instance_init(gs, new ItemInst(lua_item_get(args), args["xy"].as<Pos>()), args);
}

void lua_register_gameinst(lua_State* L) {
	luawrap::install_type<GameInst*, GameInstWrap::push, GameInstWrap::get, GameInstWrap::check>();
	luawrap::install_dynamic_casted_type<CombatGameInst*, GameInst*>();
	luawrap::install_dynamic_casted_type<PlayerInst*, GameInst*>();
	luawrap::install_dynamic_casted_type<EnemyInst*, GameInst*>();
	luawrap::install_dynamic_casted_type<FeatureInst*, GameInst*>();

	LuaValue globals = luawrap::globals(L);
	LuaValue submodule = lua_api::register_lua_submodule(L, "core.GameObject");
	submodule["enemy_create"].bind_function(enemy_create);
	submodule["feature_create"].bind_function(feature_create);
	submodule["item_create"].bind_function(item_create);
	submodule["store_create"].bind_function(store_create);

	submodule["DOOR_OPEN"] = (int)FeatureInst::DOOR_OPEN;
	submodule["DOOR_CLOSED"] = (int)FeatureInst::DOOR_CLOSED;
	submodule["PORTAL"] = (int)FeatureInst::PORTAL;
	submodule["OTHER"] = (int)FeatureInst::OTHER;
}
