#include "gamestate/GameMapState.h"
#include "gamestate/GameState.h"

#include "objects/enemy/EnemyInst.h"

#include "objects/player/PlayerInst.h"
#include "objects/GameInstRef.h"

#include "stats/items/WeaponEntry.h"

#include "stats/combat_stats.h"
#include "stats/stats.h"

#include "lua_newapi.h"
#include "lunar.h"

class CombatStatsLuaBinding {
public:
	static const char className[];
	static Lunar<CombatStatsLuaBinding>::RegType methods[];

	CombatStatsLuaBinding(GameInst* inst) :
			inst(inst) {
	}
	CombatStatsLuaBinding(const CombatStats& value) :
			value(value) {
	}

	CombatStats* get_stats(lua_State* L) {
		if (!inst.get())
			return &value;
		GameState* gs = lua_api::gamestate(L);
		CombatGameInst* combat_inst =
				dynamic_cast<CombatGameInst*>(inst.get());
		if (!combat_inst)
			return NULL;
		return &combat_inst->stats();
	}

private:
	GameInstRef inst;
	CombatStats value; //If id == 0, use this
};

typedef CombatStatsLuaBinding bind_t;
typedef Lunar<CombatStatsLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)

static int lua_member_lookup(lua_State* L) {
#define IFLUA_NUM_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(L, m );\
	}
#define IFLUA_STR_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushlstring(L, m.c_str(), m.size() );\
	}

	bind_t* state = lunar_t::check(L, 1);
	const char* cstr = lua_tostring(L, 2);
	CombatStats* stats = state->get_stats(L);

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
	else IFLUA_NUM_MEMB_LOOKUP("xp", stats->class_stats.xp)
	else IFLUA_NUM_MEMB_LOOKUP("xp_needed", stats->class_stats.xpneeded)
	else IFLUA_NUM_MEMB_LOOKUP("level", stats->class_stats.xplevel)
	else IFLUA_NUM_MEMB_LOOKUP("attack_cooldown", stats->cooldowns.action_cooldown)
	else IFLUA_STR_MEMB_LOOKUP("weapon_type", stats->equipment.weapon().weapon_entry().weapon_class)
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
	CombatStats* stats = state->get_stats(L);

	IFLUA_NUM_MEMB_UPDATE("hp", stats->core.hp)
	else IFLUA_NUM_MEMB_UPDATE("mp", stats->core.mp)
	else IFLUA_NUM_MEMB_UPDATE("max_hp", stats->core.max_hp)
	else IFLUA_NUM_MEMB_UPDATE("max_mp", stats->core.max_mp)
	else IFLUA_NUM_MEMB_UPDATE("strength", stats->core.strength)
	else IFLUA_NUM_MEMB_UPDATE("magic", stats->core.magic)
	else IFLUA_NUM_MEMB_UPDATE("defence", stats->core.defence)
	else IFLUA_NUM_MEMB_UPDATE("willpower", stats->core.willpower)
	else IFLUA_NUM_MEMB_UPDATE("speed", stats->movespeed)
	else IFLUA_NUM_MEMB_UPDATE("xp", stats->class_stats.xp)
	else IFLUA_NUM_MEMB_UPDATE("xpneeded", stats->class_stats.xpneeded)
	else IFLUA_NUM_MEMB_UPDATE("level", stats->class_stats.xplevel)
	else IFLUA_NUM_MEMB_UPDATE("attack_cooldown", stats->cooldowns.action_cooldown)
	else {
		lua_getglobal(L, bind_t::className);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, tableind);
	}
	return 1;
}

meth_t bind_t::methods[] = { meth_t(0, 0) };

void lua_combatstats_bindings(GameState* gs, lua_State* L) {
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
void lua_push_combatstats(lua_State* L, GameInst* inst) {
	lunar_t::push(L, new CombatStatsLuaBinding(inst), true);
}
void lua_push_combatstats(lua_State* L, const CombatStats& stats) {
	lunar_t::push(L, new CombatStatsLuaBinding(stats), true);
}
CombatStats& lua_get_combatstats(lua_State* L, int idx) {
	return *lunar_t::check(L, idx)->get_stats(L);
}

const char CombatStatsLuaBinding::className[] = "CombatStats";
