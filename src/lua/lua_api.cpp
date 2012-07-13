#include "lua_api.h"

extern "C" {
    #include <lua/lua.h>
    #include <lua/lauxlib.h>
    #include <lua/lualib.h>
}

void lua_gameinst_bindings(GameState* gs, lua_State* L);
void lua_effects_bindings(GameState* gs, lua_State* L);
void lua_gamestate_bindings(GameState* gs, lua_State* L);
void lua_combatstats_bindings(GameState* gs, lua_State* L);
void lua_effectivestats_bindings(GameState* gs, lua_State* L);
void lua_collision_check_bindings(lua_State* L);
void lua_spelltarget_bindings(lua_State* L);

void lua_lanarts_api(GameState* state, lua_State* L){
	luaL_openlibs(L);
	lua_gamestate_bindings(state, L);
	lua_gameinst_bindings(state, L);
//	lua_effects_bindings(state, L);
	lua_combatstats_bindings(state, L);
	lua_effectivestats_bindings(state, L);
	lua_collision_check_bindings(L);
	lua_spelltarget_bindings(L);
}

