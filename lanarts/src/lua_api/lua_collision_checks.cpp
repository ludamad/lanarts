/*
 * lua_collision_checks.cpp:
 *  Small collision check api callable form lua
 */

#include <lua.hpp>

#include <luawrap/luawrap.h>

#include "gamestate/GameState.h"

#include "objects/player/PlayerInst.h"

#include "objects/GameInst.h"

#include "lua_api.h"

// Takes obj returns bool
static int obj_solid_check(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int nargs = lua_gettop(L);

	GameInst* obj = luawrap::get<GameInst*>(L, 1);
	int x = nargs >= 2 ? lua_tointeger(L, 2) : obj->x;
	int y = nargs >= 3 ? lua_tointeger(L, 3) : obj->y;
	int radius = nargs >= 4 ? lua_tointeger(L, 4) : obj->radius;

	lua_pushboolean(L, gs->solid_test(obj, x, y, radius));
	return 1;
}

// Takes x, y, rad, [player observer] returns bool
static int radius_visible_check(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int nargs = lua_gettop(L);

	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	int radius = lua_tointeger(L, 3);
	PlayerInst* observer =
			nargs >= 4 ?
					dynamic_cast<PlayerInst*>(luawrap::get<GameInst*>(L, 4)) : NULL;

	lua_pushboolean(L, gs->radius_visible_test(x, y, radius, observer, false));
	return 1;
}

// Takes obj returns bool
static int obj_tile_check(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	GameInst* obj = luawrap::get<GameInst*>(L, 1);
	lua_pushboolean(L, gs->tile_radius_test(obj->radius, obj->x, obj->y));
	return 1;
}

// Takes obj returns bool
static int radius_tile_check(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	GameInst* obj = luawrap::get<GameInst*>(L, 1);

	int x = lua_tointeger(L, 1);
	int y = lua_tointeger(L, 2);
	int radius = lua_tointeger(L, 3);

	lua_pushboolean(L, gs->tile_radius_test(radius, x, y));
	return 1;
}

// Takes obj returns obj
static int obj_enemy_check(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	GameInst* obj = luawrap::get<GameInst*>(L, 1);
	GameInst* enemy = NULL;
	if (gs->object_radius_test(obj, &enemy, 1)) {
		luawrap::push(L, enemy);
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
	LUA_FUNC_REGISTER(radius_visible_check);
	LUA_FUNC_REGISTER(obj_enemy_check);
}
