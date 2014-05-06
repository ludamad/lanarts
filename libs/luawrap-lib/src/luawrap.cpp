/*
 * luawrap.h:
 *  Implementation of non-template functions in luawrap.h
 */

#include <cstring>

#include <lua.hpp>

#include <luawrap/luawrap.h>
#include <luawrap/luawraperror.h>

#include "luawrapassert.h"

namespace luawrap {
	void dostring(lua_State* L, const char* code) {
		if (luaL_loadstring(L, code) != 0) {
			lua_pushfstring(L, "Fatal error when parsing lua code ...\n %s \n ... \n%s\n",
					code, lua_tostring(L, -1));
			error(lua_tostring(L, -1));
			lua_pop(L, 1);
			return;
		}

		// Push error function before lua function
		lua_pushcfunction(L, luawrap::errorfunc);
		lua_insert(L, -2);
		if (lua_pcall(L, 0, 0, -2)) {
			error(lua_tostring(L, -1));
		}

		// pop error function
		lua_pop(L, 1);
	}

	LuaValue dofile(lua_State* L, const char* filename) {
		if (luaL_loadfile(L, filename) != 0) {
			lua_pushfstring(L, "Fatal error when loading lua file %s:\n%s\n",
					filename, lua_tostring(L, -1));
			error(lua_tostring(L, -1));
			lua_pop(L, 1);
			return LuaValue();
		}

		// Push error function before lua function
		lua_pushcfunction(L, luawrap::errorfunc);
		lua_insert(L, -2);
		if (lua_pcall(L, 0, 1, -2)) {
			error(lua_tostring(L, -1));
		}
		LuaValue ret = LuaValue::pop_value(L);

		// pop error function
		lua_pop(L, 1);
		return ret;
	}
}
