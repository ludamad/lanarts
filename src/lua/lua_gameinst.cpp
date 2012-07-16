#include <cstring>
#include <lua/lunar.h>

#include "../data/effect_data.h"
#include "../data/item_data.h"
#include "../data/lua_game_data.h"

#include "../gamestats/stats.h"

#include "../world/GameState.h"
#include "../world/objects/EnemyInst.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"

#include "lua_api.h"

class GameInstLuaBinding {
public:
	static const char className[];
	static Lunar<GameInstLuaBinding>::RegType methods[];

	GameInstLuaBinding(GameInst* inst) :
			inst(inst) {
	}

	GameInst* get_inst() {
		return inst.get_instance();
	}

	CombatGameInst* get_combat_inst() {
		CombatGameInst* combat_inst = dynamic_cast<CombatGameInst*>(get_inst());
		return combat_inst;
	}
	CombatStats* get_stats() {
		CombatGameInst* combat_inst = get_combat_inst();
		if (combat_inst != NULL) {
			return &combat_inst->stats();
		} else {
			return NULL;
		}
	}
	int heal_fully(lua_State* L) {
		get_stats()->core.heal_fully();
		return 0;
	}
	int direct_damage(lua_State* L) {
		get_combat_inst()->damage(lua_get_gamestate(L), lua_tonumber(L, 1));
		return 0;
	}
	int damage(lua_State* L) {
		GameState* gs = lua_get_gamestate(L);
		EffectiveAttackStats attack;
		int nargs = lua_gettop(L);
		attack.power = round(lua_tonumber(L, 1));
		attack.damage = round(lua_tonumber(L, 2));
		attack.magic_percentage = nargs >= 3 ? lua_tonumber(L, 3) : 1.0f;

		get_combat_inst()->damage(lua_get_gamestate(L), attack);
		return 0;
	}
	int equip(lua_State* L) {
		CombatGameInst* combatinst;
		if ((combatinst = dynamic_cast<CombatGameInst*>(get_inst()))) {
			int args = lua_gettop(L);

			lua_pushstring(L, "name");
			lua_gettable(L, 1);
			const char* itemname = lua_tostring(L, lua_gettop(L));
			item_id item = get_item_by_name(itemname);
			int amnt = args >= 2 ? lua_tonumber(L, 2) : 1;
			combatinst->equip(item, amnt);
			lua_pop(L, 1);
		}
		return 0;
	}
	int add_effect(lua_State* L) {
		CombatGameInst* combatinst;
		if ((combatinst = dynamic_cast<CombatGameInst*>(get_inst()))) {
			LuaValue effect = combatinst->effects().add(lua_get_gamestate(L),
					effect_from_lua(L, 1), lua_tonumber(L, 2));
			effect.push(L);
		} else {
			lua_pushnil(L);
		}

		return 1;
	}
	int move_to(lua_State* L) {
		inst->update_position(lua_tonumber(L, 1), lua_tonumber(L, 2));
		return 0;
	}
	int heal_hp(lua_State* L) {
		get_stats()->core.heal_hp(lua_tonumber(L, 1));
		return 0;
	}
	int heal_mp(lua_State* L) {
		get_stats()->core.heal_mp(lua_tonumber(L, 1));
		return 0;
	}
private:
	GameInstRef inst;
};

typedef GameInstLuaBinding bind_t;
typedef Lunar<GameInstLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)

GameInst* lua_gameinst_arg(lua_State* L, int narg) {
	bind_t* bind = lunar_t::check(L, narg);
	return bind->get_inst();
}

#define IFLUA_NUM_MEMB_LOOKUP(n, m) \
	if (strncmp(cstr, n, sizeof(n))==0){\
	lua_pushnumber(L, m );\
}
#define IFLUA_STATS_MEMB_LOOKUP(n, m) \
	if (strncmp(cstr, n, sizeof(n))==0){\
	lua_push_combatstats(L, m );\
}

static int lua_member_lookup(lua_State* L) {
	bind_t* state = lunar_t::check(L, 1);
	const char* cstr = lua_tostring(L, 2);

	GameInst* inst = state->get_inst();

	IFLUA_NUM_MEMB_LOOKUP("x", inst->x)
	else IFLUA_NUM_MEMB_LOOKUP("y", inst->y)
	else IFLUA_NUM_MEMB_LOOKUP("id", inst->id)
	else IFLUA_NUM_MEMB_LOOKUP("radius", inst->radius)
	else IFLUA_NUM_MEMB_LOOKUP("target_radius", inst->target_radius)
	else IFLUA_STATS_MEMB_LOOKUP("stats", inst)
	else {
		lua_getglobal(L, bind_t::className);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, tableind);
		lua_replace(L, tableind);
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);

			inst->lua_variables.push(L);
			tableind = lua_gettop(L);
			lua_pushvalue(L, 2);
			lua_gettable(L, tableind);
			lua_replace(L, tableind);
		}
	}
	return 1;
}
static int lua_member_update(lua_State* L) {
#define IFLUA_NUM_MEMB_UPDATE(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
			m = lua_tonumber(L, 3 );\
	}

	bind_t* state = lunar_t::check(L, 1);
	GameInst* inst = state->get_inst();
	CombatGameInst* combatinst = state->get_combat_inst();
	const char* cstr = lua_tostring(L, 2);

	bool had_member = true;
	IFLUA_NUM_MEMB_UPDATE("x", inst->x)
	else IFLUA_NUM_MEMB_UPDATE("y", inst->y)
	else if (combatinst) {
		IFLUA_NUM_MEMB_UPDATE("vx", combatinst->vx)
		else IFLUA_NUM_MEMB_UPDATE("vy", combatinst->vy)
		else
			had_member = false;
	}
	if (!had_member) {
		if (inst->lua_variables.empty())
			inst->lua_variables.table_initialize(L);
		inst->lua_variables.push(L);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
		lua_settable(L, tableind);
		lua_replace(L, tableind);
	}
	return 1;
}

meth_t bind_t::methods[] = { LUA_DEF(heal_fully), LUA_DEF(move_to),
		LUA_DEF(heal_hp), LUA_DEF(heal_mp), LUA_DEF(direct_damage),
		LUA_DEF(damage), LUA_DEF(equip), LUA_DEF(add_effect),
		LUA_DEF(add_effect), meth_t(0, 0) };

void lua_gameinst_bindings(GameState* gs, lua_State* L) {
	lunar_t::Register(L);

	luaL_getmetatable(L, bind_t::className);
//
	int tableind = lua_gettop(L);

	lua_pushstring(L, "__index");
	lua_pushcfunction(L, lua_member_lookup);
	lua_settable(L, tableind);

	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, lua_member_update);
	lua_settable(L, tableind);
}

void lua_push_gameinst(lua_State* L, GameInst* inst) {
	lunar_t::push(L, new bind_t(inst), true);
}

const char GameInstLuaBinding::className[] = "GameInst";
