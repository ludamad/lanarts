/*
 * lua_levelgen_funcs.cpp:
 *  Lua level gen callbacks
 */

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}
#include "../lua/lua_api.h"

#include "../data/lua_game_data.h"
#include "lua_levelgen_funcs.h"
#include "levelgen.h"

#include "../gamestate/GameState.h"

static GeneratedLevel* lua_togenlevel(lua_State* L, int idx) {
	return *((GeneratedLevel**)lua_touserdata(L, idx));
}

static Range lua_torange(lua_State* L, int idx) {
	Range r;
	lua_rawgeti(L, idx, 1);
	r.min = lua_tonumber(L, -1);
	lua_rawgeti(L, idx, 2);
	r.max = lua_tonumber(L, -1);
	lua_pop(L, 2);
	return r;
}

static int gen_room(lua_State* L) {
	MTwist& mt = lua_get_gamestate(L)->rng();
	int nargs = lua_gettop(L);

	GeneratedLevel* level = lua_togenlevel(L, 1);
	int room_padding = lua_tonumber(L, 2);
	Dim size(lua_tonumber(L, 3), lua_tonumber(L, 4));
	int max_attempts = nargs >= 5 ? lua_tonumber(L, 5) : 20;

	int mark = mt.rand(4);
	if (mt.rand(4) < 3)
		mark = 0;

	bool success = generate_room(lua_get_gamestate(L)->rng(), *level, size.w,
			size.h, room_padding, mark, max_attempts);

	if (success) {
		lua_push_region(L, level->rooms().back().room_region);
	} else {
		lua_pushnil(L);
	}

	return 1;
}

static int gen_monster(lua_State* L) {
	MTwist& mt = lua_get_gamestate(L)->rng();
	int nargs = lua_gettop(L);

	GeneratedLevel* level = lua_togenlevel(L, 1);
	enemy_id etype = enemy_from_lua(L, 2);
	Region r = lua_toregion(L, 3);
	int amount = nargs >= 4 ? lua_tonumber(L, 4) : 1;

	bool success = generate_enemy(*level, mt, etype, r, amount);

	lua_pushboolean(L, success);

	return 1;
}

static int gen_room_amount(lua_State* L) {
	GeneratedLevel* level = lua_togenlevel(L, 1);
	lua_pushnumber(L, level->rooms().size());
	return 1;
}

static int gen_room_get(lua_State* L) {
	GeneratedLevel* level = lua_togenlevel(L, 1);
	lua_push_region(L, level->rooms().at(lua_tonumber(L, 2)).room_region);
	return 1;
}

void lua_push_generatedlevel(lua_State* L, GeneratedLevel& level) {
	void* data = lua_newuserdata(L, sizeof(GeneratedLevel*));
	*((GeneratedLevel**)data) = &level;
}

void lua_invalidate_generatedlevel(lua_State* L, int idx) {
	void* data = lua_touserdata(L, idx);
	*((GeneratedLevel**)data) = NULL;
}

void lua_levelgen_func_bindings(lua_State* L) {
#define LUA_FUNC_REGISTER(f) \
	lua_pushcfunction(L, f); \
	lua_setfield(L, LUA_GLOBALSINDEX, #f);
	LUA_FUNC_REGISTER(gen_room);
	LUA_FUNC_REGISTER(gen_monster);
	LUA_FUNC_REGISTER(gen_room_amount);
	LUA_FUNC_REGISTER(gen_room_get);
}
