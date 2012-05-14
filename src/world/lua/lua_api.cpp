#include "lua_api.h"

extern "C" {
    #include <lua/lua.h>
    #include <lua/lauxlib.h>
    #include <lua/lualib.h>
}

void lua_lanarts_api(GameState* state, lua_State* L){
	luaL_openlibs(L);
	lua_gamestate_bindings(state, L);
	lua_gameinst_bindings(state, L);
	lua_item_bindings(state, L);

}

