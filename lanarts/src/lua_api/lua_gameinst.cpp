/*
 * lua_gameinst.cpp:
 *  Representation of a GameInst in Lua.
 */

#include <lua.hpp>

#include <lua_api/lua_newapi.h>
#include <luawrap/luawrap.h>
#include <luawrap/members.h>

#include "data/lua_game_data.h"

#include "gamestate/GameState.h"

#include "objects/GameInst.h"
#include "objects/enemy/EnemyInst.h"
#include "objects/player/PlayerInst.h"
#include "objects/CombatGameInst.h"

#include "lua_api.h"
#include "lua_gameinst.h"

/* Methods */
static void lapi_combatgameinst_heal_fully(CombatGameInst* inst) {
	inst->stats().core.heal_fully();
}

static void lapi_combatgameinst_direct_damage(LuaStackValue inst, double amount) {
	inst.as<CombatGameInst*>()->damage(lua_api::gamestate(inst), amount);
}

static int lapi_combatgameinst_damage(lua_State* L) {
	using namespace luawrap;
	GameState* gs = lua_api::gamestate(L);
	EffectiveAttackStats attack;
	int nargs = lua_gettop(L);
	attack.damage = round(lua_tointeger(L, 2));
	attack.power = round(lua_tointeger(L, 3));
	attack.magic_percentage = nargs >= 4 ? lua_tonumber(L, 4) : 1.0f;
	attack.resist_modifier = nargs >= 5 ? lua_tonumber(L, 5) : 1.0f;

	get<CombatGameInst*>(L, 1)->damage(lua_api::gamestate(L), attack);
	return 0;
}

static bool lapi_combatgameinst_melee(LuaStackValue attacker, CombatGameInst* victim) {
	Weapon w = attacker.as<CombatGameInst*>()->equipment().weapon();
	return attacker.as<CombatGameInst*>()->melee_attack(lua_api::gamestate(attacker), victim, w, true);
}

static int lapi_combatgameinst_add_effect(lua_State* L) {
	CombatGameInst* combatinst = luawrap::get<CombatGameInst*>(L, 1);
	LuaValue effect = combatinst->effects().add(lua_api::gamestate(L),
			combatinst, effect_from_lua(L, 2), lua_tointeger(L, 3));
	effect.push();

	return 1;
}

static void lapi_playerinst_reset_rest_cooldown(PlayerInst* inst) {
	inst->cooldowns().reset_rest_cooldown(REST_COOLDOWN);
}

static bool lapi_playerinst_is_local_player(PlayerInst* inst) {
	return inst->is_local_player();
}

static int lapi_combatgameinst_has_effect(lua_State* L) {
	CombatGameInst* combatinst = (CombatGameInst*)luawrap::get<GameInst*>(L, 1);
	lua_pushboolean(L, combatinst->effects().get(effect_from_lua(L, 2)) != NULL);

	return 1;
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

/* Getters/setters*/
static void lapi_gameinst_set_x(GameInst* inst, LuaStackValue field,  double x) {
	inst->update_position(x, inst->y);
}
static void lapi_gameinst_set_y(GameInst* inst, LuaStackValue field, double y) {
	inst->update_position(inst->x, y);
}

static void lapi_gameinst_set_xy(GameInst* inst, LuaStackValue field, const PosF& position) {
	inst->update_position(position.x, position.y);
}
static void lapi_gameinst_step(LuaStackValue inst) {
	inst.as<GameInst*>()->step(lua_api::gamestate(inst));
}
static void lapi_gameinst_draw(LuaStackValue inst) {
	inst.as<GameInst*>()->draw(lua_api::gamestate(inst));
}
static Pos lapi_gameinst_xy(LuaStackValue inst) {
	return inst.as<GameInst*>()->pos();
}

static int lapi_gameinst_stats(lua_State* L) {
	lua_push_combatstats(L, luawrap::get<GameInst*>(L, 1));
	return 1;
}

static std::string lapi_playerinst_name(LuaStackValue inst) {
	std::vector<PlayerDataEntry>& players = lua_api::gamestate(inst)->player_data().all_players();
	for (int i = 0; i < players.size(); i++) {
		if (players[i].player_inst.get() == inst.as<PlayerInst*>()) {
			return players[i].player_name;
		}
	}
	return std::string(); // Failed
}

static std::string lapi_playerinst_class_name(PlayerInst* inst) {
	return inst->class_stats().class_entry().name;
}

static std::vector<spell_id> lapi_playerinst_spells(PlayerInst* inst) {
	return inst->stats().spells.spell_id_list();
}

static int lapi_playerinst_kills(LuaStackValue inst) {
	return inst.as<PlayerInst*>()->score_stats().kills;
}

static int lapi_playerinst_deepest_floor(PlayerInst* inst) {
	return inst->score_stats().deepest_floor;
}

static int lapi_playerinst_deaths(PlayerInst* inst) {
	return inst->score_stats().deaths;
}

static std::string lapi_enemyinst_name(EnemyInst* inst) {
	return inst->etype().name;
}

static bool lapi_enemyinst_unique(EnemyInst* inst) {
	return inst->etype().unique;
}

static int lapi_gameinst_getter_fallback(lua_State* L) {
	LuaValue& variables = luawrap::get<GameInst*>(L, 1)->lua_variables;
	printf("Getting '%s'\n", lua_tostring(L, 2));
	if (variables.empty()) {
		lua_pushnil(L);
	} else {
		variables.push();
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
	}
	return 1;
}

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
	using namespace luawrap;

	LuaValue meta = luameta_new(L, "Font");
	meta["__isgameinst"] = true;
	LuaValue methods = luameta_constants(meta), getters = luameta_getters(meta), setters = luameta_setters(meta);

	lua_pushcfunction(L, &lapi_gameinst_getter_fallback);
	luameta_defaultgetter(meta, LuaStackValue(L, -1));
	lua_pop(L, 1);

	lua_pushcfunction(L, &lapi_gameinst_setter_fallback);
	luameta_defaultsetter(meta, LuaStackValue(L, -1));
	lua_pop(L, 1);

	bind_getter(getters["x"], &GameInst::x);
	bind_getter(getters["y"], &GameInst::y);
	bind_getter(getters["id"], &GameInst::y);
	bind_getter(getters["depth"], &GameInst::depth);
	bind_getter(getters["radius"], &GameInst::radius);
	bind_getter(getters["target_radius"], &GameInst::target_radius);
	bind_getter(getters["floor"], &GameInst::current_floor);

	getters["xy"].bind_function(lapi_gameinst_xy);
	getters["stats"].bind_function(lapi_gameinst_stats);

	setters["x"].bind_function(lapi_gameinst_set_x);
	setters["y"].bind_function(lapi_gameinst_set_y);
	setters["xy"].bind_function(lapi_gameinst_set_xy);

	methods["step"].bind_function(lapi_gameinst_step);
	methods["draw"].bind_function(lapi_gameinst_draw);

	return meta;
}

static LuaValue lua_combatgameinst_metatable(lua_State* L) {
	LuaValue meta = lua_gameinst_base_metatable(L);
	LuaValue methods = luameta_constants(meta), getters = luameta_getters(meta), setters = luameta_setters(meta);

	luawrap::bind_getter(getters["vx"], &CombatGameInst::vx);
	luawrap::bind_getter(getters["vy"], &CombatGameInst::vy);

	methods["heal_fully"].bind_function(lapi_combatgameinst_heal_fully);
	methods["damage"].bind_function(lapi_combatgameinst_damage);
	methods["direct_damage"].bind_function(lapi_combatgameinst_direct_damage);
	methods["melee"].bind_function(lapi_combatgameinst_melee);
	methods["heal_hp"].bind_function(lapi_combatgameinst_heal_hp);
	methods["heal_mp"].bind_function(lapi_combatgameinst_heal_mp);

	methods["add_effect"].bind_function(lapi_combatgameinst_add_effect);
	methods["has_effect"].bind_function(lapi_combatgameinst_has_effect);

	return meta;
}

static LuaValue lua_enemyinst_metatable(lua_State* L) {
	LuaValue meta = lua_combatgameinst_metatable(L);
	LuaValue methods = luameta_constants(meta), getters = luameta_getters(meta), setters = luameta_setters(meta);

	getters["name"].bind_function(lapi_enemyinst_name);
	getters["unique"].bind_function(lapi_enemyinst_unique);

	return meta;
}

static LuaValue lua_playerinst_metatable(lua_State* L) {
	LuaValue meta = lua_combatgameinst_metatable(L);
	LuaValue methods = luameta_constants(meta), getters = luameta_getters(meta), setters = luameta_setters(meta);

	getters["name"].bind_function(lapi_playerinst_name);
	getters["class_name"].bind_function(lapi_playerinst_class_name);
	getters["kills"].bind_function(lapi_playerinst_kills);
	getters["deepest_floor"].bind_function(lapi_playerinst_deepest_floor);
	getters["deaths"].bind_function(lapi_playerinst_deaths);
	getters["spells"].bind_function(lapi_playerinst_spells);

	methods["is_local_player"].bind_function(lapi_playerinst_is_local_player);
	methods["reset_rest_cooldown"].bind_function(lapi_playerinst_reset_rest_cooldown);
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

/* Functions required by luawrap */

/* For GameInst* */
void lua_pushgameinst(lua_State* L, GameInst* inst) {
	GameInst** lua_inst = (GameInst**) lua_newuserdata(L, sizeof(GameInst*));
	*lua_inst = inst;
	lua_gameinst_push_metatable(L, inst);
	lua_setmetatable(L, -2);
}

GameInst* lua_getgameinst(lua_State* L, int idx) {
	return *(GameInst**) lua_touserdata(L, idx);
}

bool lua_checkgameinst(lua_State* L, int idx) {
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

/* For CombatGameInst* */
void lua_pushcombatgameinst(lua_State* L, CombatGameInst* inst) {
	lua_pushgameinst(L, inst);
}

CombatGameInst* lua_getcombatgameinst(lua_State* L, int idx) {
	return *(CombatGameInst**) lua_touserdata(L, idx);
}

/* For EnemyInst* */
void lua_pushenemyinst(lua_State* L, EnemyInst* inst) {
	lua_pushgameinst(L, inst);
}

EnemyInst* lua_getenemyinst(lua_State* L, int idx) {
	return *(EnemyInst**) lua_touserdata(L, idx);
}

/* For PlayerInst* */
void lua_pushplayerinst(lua_State* L, PlayerInst* inst) {
	lua_pushgameinst(L, inst);
}

PlayerInst* lua_getplayerinst(lua_State* L, int idx) {
	return *(PlayerInst**) lua_touserdata(L, idx);
}

/* TODO: Deprecated */
void lua_gameinst_callback(lua_State* L, LuaValue& value, GameInst* inst) {
	if (value.empty())
		return;
	value.push();
	luawrap::push(L, inst);
	lua_call(L, 1, 0);
}

void lua_register_gameinst(lua_State* L) {
	luawrap::install_type<GameInst*, lua_pushgameinst, lua_getgameinst,
			lua_checkgameinst>();
	luawrap::install_type<CombatGameInst*, lua_pushcombatgameinst, lua_getcombatgameinst,
			lua_checkgameinst>();
	luawrap::install_type<PlayerInst*, lua_pushplayerinst, lua_getplayerinst,
			lua_checkgameinst>();
	luawrap::install_type<EnemyInst*, lua_pushenemyinst, lua_getenemyinst,
			lua_checkgameinst>();
}
