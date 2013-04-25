#include <stdexcept>

#include <lcommon/lua_lcommon.h>
#include <lcommon/strformat.h>

#include <ldraw/lua_ldraw.h>

#include "gamestate/GameState.h"

#include <lua.hpp>

#include "lua_api/lua_newapi.h"

#include "lua_api.h"

void lua_gameinst_bindings(GameState* gs, lua_State* L);
void lua_gamestate_bindings(GameState* gs, lua_State* L);
void lua_combatstats_bindings(GameState* gs, lua_State* L);
void lua_effectivestats_bindings(GameState* gs, lua_State* L);
void lua_collision_check_bindings(lua_State* L);
void lua_spelltarget_bindings(lua_State* L);
void lua_object_func_bindings(lua_State* L);

int rand_range(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int nargs = lua_gettop(L);
	int min = 0, max = 0;
	if (nargs == 1) {
		// array[1]
		lua_rawgeti(L, 1, 1);
		min = lua_tointeger(L, -1);
		// array[2]
		lua_rawgeti(L, 1, 2);
		max = lua_tointeger(L, -1);
		lua_pop(L, 2);
	} else if (nargs > 1) {
		min = lua_tointeger(L, 1);
		max = lua_tointeger(L, 2);
	}

	lua_pushnumber(L, gs->rng().rand(min, max + 1));
	return 1;
}

static int lua_lanarts_panic(lua_State* L) {
	throw std::runtime_error(format("LUA PANIC: %s", lua_tostring(L, -1)));
	return 0;
}

void lua_lanarts_api(GameState* state, lua_State* L) {
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_lcommon(L, globals);
	ldraw::lua_register_ldraw(L, globals);

	lua_atpanic(L, lua_lanarts_panic);

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

void lua_push_narray(lua_State* L, const int* nums, int n) {
	lua_newtable(L);
	int idx = lua_gettop(L);
	for (int i = 0; i < n; i++) {
		lua_pushnumber(L, nums[i]);
		lua_rawseti(L, idx, i + 1);
	}
}
void lua_tonarray(lua_State* L, int idx, int* nums, int n) {
	for (int i = 0; i < n; i++) {
		lua_rawgeti(L, idx, i + 1);
		if (!lua_isnil(L, -1)) {
			nums[i] = lua_tointeger(L, -1);
		}
		lua_pop(L, 1);
	}
}
