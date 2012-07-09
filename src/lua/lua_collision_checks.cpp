/*
 * lua_collision_checks.cpp:
 *  Small collision check api callable form lua
 */

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

#include "lua_api.h"

#include "../world/GameState.h"

#include "../world/objects/GameInst.h"

// Takes obj returns bool
static int obj_solid_check(lua_State* L) {
	GameState* gs = lua_get_gamestate(L);
	GameInst* obj = lua_gameinst_arg(L, 1);
	lua_pushboolean(L, gs->solid_test(obj));
	return 1;
}

// Takes obj returns bool
static int obj_tile_check(lua_State* L) {
	GameState* gs = lua_get_gamestate(L);
	GameInst* obj = lua_gameinst_arg(L, 1);
	lua_pushboolean(L, gs->tile_radius_test(obj->radius, obj->x, obj->y));
	return 1;
}

// Takes obj returns bool
static int radius_tile_check(lua_State* L) {
	GameState* gs = lua_get_gamestate(L);
	GameInst* obj = lua_gameinst_arg(L, 1);
	lua_pushboolean(L, gs->tile_radius_test(obj->radius, obj->x, obj->y));
	return 1;
}

// Takes obj returns obj
static int obj_enemy_check(lua_State* L) {
	GameState* gs = lua_get_gamestate(L);
	GameInst* obj = lua_gameinst_arg(L, 1);
	GameInst* enemy = NULL;
	if (gs->object_radius_test(obj, &enemy, 1)) {
		lua_push_gameinst(L, enemy);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

void lua_collision_check_bindings(lua_State* L) {
	//Use C function name as lua function name:
#define LUA_FUNC_REGISTER(f) \
	lua_pushcfunction(L, f); \
	lua_setfield(L, LUA_GLOBALSINDEX, #f);

	LUA_FUNC_REGISTER(obj_solid_check);
	LUA_FUNC_REGISTER(obj_tile_check);
	LUA_FUNC_REGISTER(radius_tile_check);
	LUA_FUNC_REGISTER(obj_enemy_check);
}
