#include <lua/lunar.h>

#include "lua_api.h"

#include "../world/GameState.h"
#include "../world/GameLevelState.h"

#include "../world/objects/EnemyInst.h"
#include "../world/objects/PlayerInst.h"

static Stats* stats_from_inst(GameInst* inst) {
	PlayerInst* p;
	EnemyInst* e;
	if ((p = dynamic_cast<PlayerInst*>(inst))) {
		return &p->stats();
	} else if ((e = dynamic_cast<EnemyInst*>(inst))) {
		return &e->stats();
	} else {
		return NULL;
	}
}

class StatsLuaBinding {
public:
	static const char className[];
	static Lunar<StatsLuaBinding>::RegType methods[];

	StatsLuaBinding(obj_id id) :
			id(id) {
	}
	StatsLuaBinding(const Stats& value) :
			id(0), value(value) {
	}

	Stats* get_stats(lua_State* L) {
		if (!id)
			return &value;
		GameState* gs = lua_get_gamestate(L);
		GameInst* inst = gs->get_instance(id);
		return stats_from_inst(inst);
	}

private:
	obj_id id; //If id !=0, use object stats
	Stats value; //If id == 0, use this
};

typedef StatsLuaBinding bind_t;
typedef Lunar<StatsLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)

static int lua_member_lookup(lua_State* L){
	#define IFLUA_NUM_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(L, m );\
	}

	bind_t* state = lunar_t::check(L,1);
	const char* cstr = lua_tostring(L, 2);
	Stats* stats = state->get_stats(L);

	IFLUA_NUM_MEMB_LOOKUP("hp", stats->hp)
	else IFLUA_NUM_MEMB_LOOKUP("mp", stats->mp)
	else IFLUA_NUM_MEMB_LOOKUP("max_hp", stats->max_hp)
	else IFLUA_NUM_MEMB_LOOKUP("max_mp", stats->max_mp)
	else IFLUA_NUM_MEMB_LOOKUP("magic", stats->magic)
	else IFLUA_NUM_MEMB_LOOKUP("strength", stats->strength)
	else IFLUA_NUM_MEMB_LOOKUP("defence", stats->defence)
	else IFLUA_NUM_MEMB_LOOKUP("cooldown", stats->cooldown)
	else IFLUA_NUM_MEMB_LOOKUP("xp", stats->xp)
	else IFLUA_NUM_MEMB_LOOKUP("xpneeded", stats->xpneeded)
	else IFLUA_NUM_MEMB_LOOKUP("level", stats->xplevel)
	else{
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
	Stats* stats = state->get_stats(L);

	IFLUA_NUM_MEMB_UPDATE("hp", stats->hp)
	else IFLUA_NUM_MEMB_UPDATE("mp", stats->mp)
	else IFLUA_NUM_MEMB_UPDATE("max_hp", stats->max_hp)
	else IFLUA_NUM_MEMB_UPDATE("max_mp", stats->max_mp)
	else IFLUA_NUM_MEMB_UPDATE("magic", stats->magic)
	else IFLUA_NUM_MEMB_UPDATE("strength", stats->strength)
	else IFLUA_NUM_MEMB_UPDATE("defence", stats->defence)
	else IFLUA_NUM_MEMB_UPDATE("cooldown", stats->cooldown)
	else IFLUA_NUM_MEMB_UPDATE("xp", stats->xp)
	else IFLUA_NUM_MEMB_UPDATE("xpneeded", stats->xpneeded)
	else {
		lua_getglobal(L, bind_t::className);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, tableind);
	}
	return 1;
}

meth_t bind_t::methods[] = { meth_t(0, 0) };

void lua_stats_bindings(Stats* gs, lua_State* L) {
	lunar_t::Register(L);

   luaL_getmetatable(L, bind_t::className);

   int tableind = lua_gettop(L);

   lua_pushstring(L, "__index");
   lua_pushcfunction(L, lua_member_lookup);
   lua_pushstring(L, "__newIndex");
   lua_pushcfunction(L, lua_member_update);
   lua_settable(L, tableind);
}
void lua_pushgamestats(lua_State* L, obj_id id){
	lunar_t::push(L, new StatsLuaBinding(id), true);
}
const char StatsLuaBinding::className[] = "Stats";
