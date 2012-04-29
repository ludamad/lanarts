#include "lua_api.h"

extern "C" {
    #include <lua/lua.h>
    #include <lua/lauxlib.h>
    #include <lua/lualib.h>
}

void lua_lanarts_api(GameState* state, lua_State* lua_state){
	luaL_openlibs(lua_state);
	lua_gamestate_bindings(state, lua_state);
	lua_gameinst_bindings(state, lua_state);
	lua_item_bindings(state, lua_state);

}

