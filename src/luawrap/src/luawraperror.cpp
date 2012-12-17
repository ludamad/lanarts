/*
 * luawraperror.cpp:
 *  Error functions used on conversion failure and lua runtime failure.
 */

#include <lua.hpp>

#include <luawrap/luawraperror.h>

namespace luawrap {
	int errorfunc(lua_State *L) {
		if (!lua_isstring(L, 1)) /* 'message' not a string? */
			return 1; /* keep it intact */
		lua_getfield(L, LUA_GLOBALSINDEX, "debug");
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			return 1;
		}
		lua_getfield(L, -1, "traceback");
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 2);
			return 1;
		}
		lua_pushvalue(L, 1); /* pass error message */
		lua_pushinteger(L, 2); /* skip this function and traceback */
		lua_call(L, 2, 1); /* call debug.traceback */
		return 1;
	}

}
