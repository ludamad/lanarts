#include <stdexcept>

#include <common/lua_lcommon.h>
#include <draw/lua_ldraw.h>

#include "../gamestate/GameState.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}
#include "../levelgen/lua_levelgen_funcs.h"

#include "lua_api.h"

void lua_gameinst_bindings(GameState* gs, lua_State* L);
void lua_gamestate_bindings(GameState* gs, lua_State* L);
void lua_combatstats_bindings(GameState* gs, lua_State* L);
void lua_effectivestats_bindings(GameState* gs, lua_State* L);
void lua_collision_check_bindings(lua_State* L);
void lua_spelltarget_bindings(lua_State* L);
void lua_object_func_bindings(lua_State* L);
void lua_display_func_bindings(lua_State* L);

int rand_range(lua_State* L) {
	GameState* gs = lua_get_gamestate(L);
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
	throw std::runtime_error("Lanarts Lua panic!");
	return 0;
}

void lua_lanarts_api(GameState* state, lua_State* L) {
	luaL_openlibs(L);
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_lcommon(L, globals);
	ldraw::lua_register_ldraw(L, globals);

	lua_atpanic(L, lua_lanarts_panic);

	lua_gamestate_bindings(state, L);
	lua_gameinst_bindings(state, L);
	lua_combatstats_bindings(state, L);
	lua_display_func_bindings(L);
	lua_effectivestats_bindings(state, L);
	lua_collision_check_bindings(L);
	lua_spelltarget_bindings(L);
	lua_object_func_bindings(L);
	lua_levelgen_func_bindings(L);

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

void lua_push_region(lua_State* L, const Region& r) {
	lua_push_narray(L, (const int*)&r, sizeof(Region) / sizeof(int));
}

Region lua_toregion(lua_State* L, int idx) {
	Region r;
	lua_tonarray(L, idx, (int*)&r, sizeof(Region) / sizeof(int));
	return r;
}

void lua_push_colour(lua_State* L, const Colour& c) {
	lua_push_narray(L, (const int*)&c, sizeof(Colour) / sizeof(int));
}

Colour lua_tocolour(lua_State* L, int idx) {
	Colour c;
	lua_tonarray(L, idx, (int*)&c, sizeof(Colour) / sizeof(int));
	return c;
}

std::string lua_tocppstring(lua_State* L, int idx) {
	size_t size;
	const char* cstr = lua_tolstring(L, idx, &size);
	return std::string(cstr, size);
}
