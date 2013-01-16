/*
 * lua_effectivestats.cpp
 *
 *  Created on: Jun 9, 2012
 *      Author: 100397561
 */

#include "gamestate/GameLevelState.h"
#include "gamestate/GameState.h"

#include "objects/enemy/EnemyInst.h"
#include "objects/player/PlayerInst.h"

#include "stats/combat_stats.h"
#include "stats/stats.h"

#include "lua_api.h"
#include "lunar.h"

class EffectiveStatsLuaBinding {
public:
	static const char className[];
	static Lunar<EffectiveStatsLuaBinding>::RegType methods[];

	EffectiveStatsLuaBinding(GameInst* inst) :
			inst(inst) {
	}
	EffectiveStatsLuaBinding(const EffectiveStats& value) :
			value(value) {
	}

	EffectiveStats* get_stats(lua_State* L) {
		if (!inst.get())
			return &value;
		GameState* gs = lua_api::gamestate(L);
		CombatGameInst* combat_inst =
				dynamic_cast<CombatGameInst*>(inst.get());
		if (!combat_inst)
			return NULL;
		return &combat_inst->effective_stats();
	}

private:
	GameInstRef inst; //If inst != NULL, use object stats
	EffectiveStats value; //If id == 0, use this
};

typedef EffectiveStatsLuaBinding bind_t;
typedef Lunar<EffectiveStatsLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)

static int lua_member_lookup(lua_State* L) {
#define IFLUA_NUM_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(L, m );\
	}

	bind_t* state = lunar_t::check(L, 1);
	const char* cstr = lua_tostring(L, 2);
	EffectiveStats* stats = state->get_stats(L);

	if (!stats) {
		lua_pushnil(L);
		return 1;
	}

	IFLUA_NUM_MEMB_LOOKUP("hp", stats->core.hp)
	else IFLUA_NUM_MEMB_LOOKUP("mp", stats->core.mp)
	else IFLUA_NUM_MEMB_LOOKUP("max_hp", stats->core.max_hp)
	else IFLUA_NUM_MEMB_LOOKUP("max_mp", stats->core.max_mp)
	else IFLUA_NUM_MEMB_LOOKUP("strength", stats->core.strength)
	else IFLUA_NUM_MEMB_LOOKUP("magic", stats->core.magic)
	else IFLUA_NUM_MEMB_LOOKUP("defence", stats->core.defence)
	else IFLUA_NUM_MEMB_LOOKUP("willpower", stats->core.willpower)
	else IFLUA_NUM_MEMB_LOOKUP("speed", stats->movespeed)
	else IFLUA_NUM_MEMB_LOOKUP("damage", stats->physical.damage)
	else IFLUA_NUM_MEMB_LOOKUP("cooldown_mult", stats->cooldown_mult)
	else IFLUA_NUM_MEMB_LOOKUP("melee_cooldown_multiplier",
			stats->cooldown_modifiers.melee_cooldown_multiplier)
	else IFLUA_NUM_MEMB_LOOKUP("ranged_cooldown_multiplier",
			stats->cooldown_modifiers.ranged_cooldown_multiplier)
	else IFLUA_NUM_MEMB_LOOKUP("spell_cooldown_multiplier",
			stats->cooldown_modifiers.spell_cooldown_multiplier)
	else IFLUA_NUM_MEMB_LOOKUP("power", stats->physical.power)
	else IFLUA_NUM_MEMB_LOOKUP("reduction", stats->physical.reduction)
	else IFLUA_NUM_MEMB_LOOKUP("resistance", stats->physical.resistance)
	else IFLUA_NUM_MEMB_LOOKUP("magic_damage", stats->magic.damage)
	else IFLUA_NUM_MEMB_LOOKUP("magic_power", stats->magic.power)
	else IFLUA_NUM_MEMB_LOOKUP("magic_reduction", stats->magic.reduction)
	else IFLUA_NUM_MEMB_LOOKUP("magic_resistance", stats->magic.resistance)
	else {
		lua_getglobal(L, bind_t::className);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, tableind);
		lua_replace(L, tableind);
	}
	return 1;
}
static int lua_member_update(lua_State* L) {
#define IFLUA_NUM_MEMB_UPDATE(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
			m = lua_tonumber(L, 3 );\
	}

	bind_t* state = lunar_t::check(L, 1);
	const char* cstr = lua_tostring(L, 2);
	EffectiveStats* stats = state->get_stats(L);

	IFLUA_NUM_MEMB_UPDATE("hp", stats->core.hp)
	else IFLUA_NUM_MEMB_UPDATE("mp", stats->core.mp)
	else IFLUA_NUM_MEMB_UPDATE("max_hp", stats->core.max_hp)
	else IFLUA_NUM_MEMB_UPDATE("max_mp", stats->core.max_mp)
	else IFLUA_NUM_MEMB_UPDATE("strength", stats->core.strength)
	else IFLUA_NUM_MEMB_UPDATE("magic", stats->core.magic)
	else IFLUA_NUM_MEMB_UPDATE("defence", stats->core.defence)
	else IFLUA_NUM_MEMB_UPDATE("willpower", stats->core.willpower)
	else IFLUA_NUM_MEMB_UPDATE("speed", stats->movespeed)
	else IFLUA_NUM_MEMB_UPDATE("cooldown_mult", stats->cooldown_mult)
	else IFLUA_NUM_MEMB_UPDATE("melee_cooldown_multiplier",
			stats->cooldown_modifiers.melee_cooldown_multiplier)
	else IFLUA_NUM_MEMB_UPDATE("ranged_cooldown_multiplier",
			stats->cooldown_modifiers.ranged_cooldown_multiplier)
	else IFLUA_NUM_MEMB_UPDATE("spell_cooldown_multiplier",
			stats->cooldown_modifiers.spell_cooldown_multiplier)
	else IFLUA_NUM_MEMB_UPDATE("damage", stats->physical.damage)
	else IFLUA_NUM_MEMB_UPDATE("power", stats->physical.power)
	else IFLUA_NUM_MEMB_UPDATE("reduction", stats->physical.reduction)
	else IFLUA_NUM_MEMB_UPDATE("resistance", stats->physical.resistance)
	else IFLUA_NUM_MEMB_UPDATE("magic_damage", stats->magic.damage)
	else IFLUA_NUM_MEMB_UPDATE("magic_power", stats->magic.power)
	else IFLUA_NUM_MEMB_UPDATE("magic_reduction", stats->magic.reduction)
	else IFLUA_NUM_MEMB_UPDATE("magic_resistance", stats->magic.resistance)
	else {
		lua_getglobal(L, bind_t::className);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, tableind);
	}
	return 1;
}

meth_t bind_t::methods[] = { meth_t(0, 0) };

void lua_effectivestats_bindings(GameState* gs, lua_State* L) {
	lunar_t::Register(L);

	luaL_getmetatable(L, bind_t::className);

	int tableind = lua_gettop(L);

	lua_pushstring(L, "__index");
	lua_pushcfunction(L, lua_member_lookup);
	lua_settable(L, tableind);
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, lua_member_update);
	lua_settable(L, tableind);
}
void lua_push_effectivestats(lua_State* L, GameInst* inst) {
	lunar_t::push(L, new EffectiveStatsLuaBinding(inst), true);
}
void lua_push_effectivestats(lua_State* L, const EffectiveStats& stats) {
	lunar_t::push(L, new EffectiveStatsLuaBinding(stats), true);
}

void lua_push_effectiveattackstats(lua_State* L,
		const EffectiveAttackStats& stats) {
	lua_newtable(L);
	int tableidx = lua_gettop(L);
	lua_pushnumber(L, stats.damage);
	lua_setfield(L, tableidx, "damage");
	lua_pushnumber(L, stats.power);
	lua_setfield(L, tableidx, "power");
	lua_pushnumber(L, stats.magic_percentage);
	lua_setfield(L, tableidx, "magic_percentage");
	lua_pushnumber(L, 1.0 - stats.magic_percentage);
	lua_setfield(L, tableidx, "physical_percentage");
}
EffectiveStats& lua_get_effectivestats(lua_State* L, int idx) {
	return *lunar_t::check(L, idx)->get_stats(L);
}
const char EffectiveStatsLuaBinding::className[] = "EffectiveStats";
