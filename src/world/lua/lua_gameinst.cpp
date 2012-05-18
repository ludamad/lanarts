#include "lua_api.h"
#include <cstring>

#include <lua/lunar.h>
#include "../GameState.h"
#include "../objects/EnemyInst.h"
#include "../objects/GameInst.h"
#include "../objects/PlayerInst.h"
#include "../../gamestats/Stats.h"
#include "../../data/item_data.h"



class GameInstLuaBinding {
public:
  static const char className[];
  static Lunar<GameInstLuaBinding>::RegType methods[];

  GameInstLuaBinding(lua_State* L, obj_id id) :
	  gs(lua_get_gamestate(L)), id(id) {
  }

  GameInst* get_instance() {
	  return gs->get_instance(id);
  }
  Stats* get_stats() {
	  GameInst* g = get_instance();
	  PlayerInst* p;
	  EnemyInst* e;
	  if ( (p = dynamic_cast<PlayerInst*>(g))){
		  return &p->stats();
	  } else if ( (e = dynamic_cast<EnemyInst*>(g))){
		  return &e->stats();
	  } else {
		  return NULL;
	  }
  }
  int heal_fully(lua_State* L){
	  get_stats()->heal_fully();
	  return 0;
  }
  int hurt(lua_State* L){
	  get_stats()->hurt(lua_tonumber(L,1));
	  return 0;
  }
  int equip(lua_State* L){
	  PlayerInst* p;
	  if ( (p = dynamic_cast<PlayerInst*>(get_instance()))){
		  p->weapon_type() = lua_item_arg(L,1).weapon;
	  }
	  return 0;
  }
  int hasten(lua_State* L){
	  PlayerInst* p;
	  if ( (p = dynamic_cast<PlayerInst*>(get_instance()))){
		p->status_effects().add(EFFECT_HASTE, lua_tonumber(L, 1));
	  }
	  return 0;
  }
  int heal_hp(lua_State* L){
	  get_stats()->raise_hp(lua_tonumber(L,1));
	  return 0;
  }
  int heal_mp(lua_State* L){
	  get_stats()->raise_mp(lua_tonumber(L,1));
	  return 0;
  }
  obj_id get_id(){ return id; }
private:
  GameState* gs;
  obj_id id;
};


typedef GameInstLuaBinding bind_t;
typedef Lunar<GameInstLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)

obj_id lua_gameinst_arg(lua_State* L, int narg){
	bind_t* bind = lunar_t::check(L, narg);
	return bind->get_id();
}
static int lua_member_lookup(lua_State* L){
	#define IFLUA_NUM_MEMB_LOOKUP(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(L, m );\
	}

	bind_t* state = lunar_t::check(L,1);
	GameInst* inst = state->get_instance();
	Stats* stats = state->get_stats();
	const char* cstr = lua_tostring(L, 2);

	IFLUA_NUM_MEMB_LOOKUP("hp", stats->hp)
	else IFLUA_NUM_MEMB_LOOKUP("mp", stats->mp)
	else IFLUA_NUM_MEMB_LOOKUP("max_hp", stats->max_hp)
	else IFLUA_NUM_MEMB_LOOKUP("max_mp", stats->max_mp)
	else IFLUA_NUM_MEMB_LOOKUP("x", inst->x)
	else IFLUA_NUM_MEMB_LOOKUP("y", inst->y)
	else IFLUA_NUM_MEMB_LOOKUP("magic", stats->magic)
	else IFLUA_NUM_MEMB_LOOKUP("strength", stats->strength)
	else IFLUA_NUM_MEMB_LOOKUP("defence", stats->defence)
	else IFLUA_NUM_MEMB_LOOKUP("xp", stats->xp)
	else IFLUA_NUM_MEMB_LOOKUP("xpneeded", stats->xpneeded)
	else IFLUA_NUM_MEMB_LOOKUP("level", stats->xplevel)
	else IFLUA_NUM_MEMB_LOOKUP("id", inst->id)
	else{
		lua_getglobal(L, bind_t::className);
		int tableind = lua_gettop(L);
		lua_pushvalue(L, 2);
		lua_gettable(L, tableind);
	}
	return 1;
}
static int lua_member_update(lua_State* L){
	#define IFLUA_NUM_MEMB_UPDATE(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
			m = lua_tonumber(L, 3 );\
	}

	bind_t* state = lunar_t::check(L,1);
	GameInst* inst = state->get_instance();
	Stats* stats = state->get_stats();
	const char* cstr = lua_tostring(L, 2);

	IFLUA_NUM_MEMB_UPDATE("hp", stats->hp)
	else IFLUA_NUM_MEMB_UPDATE("mp", stats->mp)
	else IFLUA_NUM_MEMB_UPDATE("magic", stats->magic)
	else IFLUA_NUM_MEMB_UPDATE("strength", stats->strength)
	else IFLUA_NUM_MEMB_UPDATE("defence", stats->defence)
	else IFLUA_NUM_MEMB_UPDATE("xp", stats->xp)
	else IFLUA_NUM_MEMB_UPDATE("x", inst->x)
	else IFLUA_NUM_MEMB_UPDATE("y", inst->y)
	else {
		if (inst->lua_variables.empty())
			inst->lua_variables.table_initialize(L);
//		inst->lua_variables.set_value(L, cstr);
	}
	return 1;
}


meth_t bind_t::methods[] = {
		LUA_DEF(heal_fully),
		LUA_DEF(heal_hp),
		LUA_DEF(heal_mp),
		LUA_DEF(hurt),
		LUA_DEF(equip),
		LUA_DEF(hasten),
  meth_t(0,0)
};


void lua_gameinst_bindings(GameState* gs, lua_State* L){
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


void lua_pushgameinst(lua_State* L, obj_id id){
	lunar_t::push(L, new GameInstLuaBinding(L, id), true);
}
const char GameInstLuaBinding::className[] = "GameInst";
