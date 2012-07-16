#include "lua_api.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}
#include "../world/GameState.h"

void lua_gameinst_bindings(GameState* gs, lua_State* L);
void lua_gamestate_bindings(GameState* gs, lua_State* L);
void lua_combatstats_bindings(GameState* gs, lua_State* L);
void lua_effectivestats_bindings(GameState* gs, lua_State* L);
void lua_collision_check_bindings(lua_State* L);
void lua_spelltarget_bindings(lua_State* L);
void lua_object_func_bindings(lua_State* L);

int rand_range(lua_State* L) {
	GameState* gs = lua_get_gamestate(L);
	int nargs = lua_gettop(L);

	lua_pushnumber(L, 1.0);
	lua_rawget(L, 1);
	int min = lua_tonumber(L, lua_gettop(L));
	int max = min;
	lua_pop(L, 1);

	if (nargs > 1) {
		lua_pushnumber(L, 2.0);
		lua_rawget(L, 1);
		max = lua_tonumber(L, lua_gettop(L));
		lua_pop(L, 1);
	}

	lua_pushnumber(L, gs->rng().rand(min, max + 1));
	return 1;
}

void lua_lanarts_api(GameState* state, lua_State* L) {
	luaL_openlibs(L);
	lua_gamestate_bindings(state, L);
	lua_gameinst_bindings(state, L);
	lua_combatstats_bindings(state, L);
	lua_effectivestats_bindings(state, L);
	lua_collision_check_bindings(L);
	lua_spelltarget_bindings(L);
	lua_object_func_bindings(L);

	//Use C function name as lua function name:
#define LUA_FUNC_REGISTER(f) \
	lua_pushcfunction(L, f); \
	lua_setfield(L, LUA_GLOBALSINDEX, #f);
	LUA_FUNC_REGISTER(rand_range);
}

