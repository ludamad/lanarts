/*
 * lua_array_util.cpp:
 *  Utilities for dealing with array-like lua tables.
 */

#include <lua.hpp>

#include "lanarts_defines.h"
#include "lua_array_util.h"

void luaarray_append(const LuaField& array, const LuaField& value) {
	LANARTS_ASSERT(array.luastate() == value.luastate());

	lua_State* L = array.luastate();
	array.push();
	value.push();
	lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
	lua_pop(L, 1);
}

// Clearing is unfortunately an O(N) operation
void luaarray_clear(const LuaField& array) {
	lua_State* L = array.luastate();

	array.push();

	int len = lua_objlen(L, -1);
	do {
		lua_pushnil(L);
		lua_rawseti(L, -2, len);
	} while (--len);
	lua_pop(L, 1);
}
