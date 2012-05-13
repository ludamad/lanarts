#include "lua_api.h"

#include <lua/lunar.h>
#include "../GameState.h"
#include "../GameLevelState.h"


class GameStateLuaBinding {
public:
  static const char className[];
  static Lunar<GameStateLuaBinding>::RegType methods[];

  GameStateLuaBinding(GameState* gs) : gs(gs) {
  }

  int destroy_object(lua_State* lua_state){
	  obj_id id = lua_gameinst_arg(lua_state, 1);
	  if (id > 0){
		  gs->remove_instance(gs->get_instance(id));
	  }
	  return 0;
  }

  int players_in_room(lua_State* lua_state){
	  PlayerController& pc = gs->player_controller();
	  lua_createtable(lua_state, 0, 0);
	  int table = lua_gettop(lua_state);
	  for (int i=0; i<pc.player_ids().size(); i++) {
		  lua_pushgameinst(lua_state, pc.player_ids()[i]);
		//  lua_pushnumber(lua_state, 2);
		  lua_rawseti (lua_state, table, i+1);
	  }
	  return 1;
  }
  GameState* game_state(){
	  return gs;
  }
private:
  GameState* gs;
};

typedef GameStateLuaBinding bind_t;
typedef Lunar<GameStateLuaBinding> lunar_t;
typedef lunar_t::RegType meth_t;
#define LUA_DEF(m) meth_t(#m, &bind_t:: m)

static int lua_member_lookup(lua_State* lua_state){
	#define IFLUA_NUM_MEMB_UPDATE(n, m) \
		if (strncmp(cstr, n, sizeof(n))==0){\
		lua_pushnumber(lua_state, m );\
	}

	bind_t* state = lunar_t::check(lua_state,1);
	GameState* gs = state->game_state();
	const char* cstr = lua_tostring(lua_state, 2);

	IFLUA_NUM_MEMB_UPDATE("width", gs->width())
	else IFLUA_NUM_MEMB_UPDATE("height", gs->height())
	else IFLUA_NUM_MEMB_UPDATE("mouse_x", gs->mouse_x())
	else IFLUA_NUM_MEMB_UPDATE("mouse_y", gs->mouse_y())
	else IFLUA_NUM_MEMB_UPDATE("frame_number", gs->frame())
	else IFLUA_NUM_MEMB_UPDATE("level_number", gs->level()->level_number)
	else IFLUA_NUM_MEMB_UPDATE("monster_num", gs->monster_controller().number_monsters())
	else {
		lua_getglobal(lua_state, bind_t::className);
		int tableind = lua_gettop(lua_state);
		lua_pushvalue(lua_state, 2);
		lua_gettable(lua_state, tableind);
	}
	return 1;
}


meth_t bind_t::methods[] = {
  LUA_DEF(destroy_object),
  LUA_DEF(players_in_room),
  meth_t(0,0)
};


GameState* lua_get_gamestate(lua_State* lua_state){
	lua_getglobal(lua_state, "world");
	int idx = lua_gettop(lua_state);
	bind_t* obj = lunar_t::check(lua_state, idx);
	lua_pop(lua_state, 1);
	if (!obj) return NULL;
	return obj->game_state();
}

void lua_gamestate_bindings(GameState* gs, lua_State* lua_state){
	lunar_t::Register(lua_state);

     luaL_getmetatable(lua_state, bind_t::className);
//
    int tableind = lua_gettop(lua_state);

    lua_pushstring(lua_state, "__index");
    lua_pushcfunction(lua_state, lua_member_lookup);
    lua_settable(lua_state, tableind);

	lunar_t::push(lua_state, new bind_t(gs), true);

	lua_setglobal(lua_state, "world");
}
const char GameStateLuaBinding::className[] = "GameWorld";
