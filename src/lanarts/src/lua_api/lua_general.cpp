/*
 * lua_general.cpp:
 *  General utility functions.
 */
#include <lua.hpp>

#include <lcommon/mathutil.h>

#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

#include "util/math_util.h"

static int lapi_values_aux(lua_State* L) {
	long idx = (long)lua_touserdata(L, lua_upvalueindex(2));
	long len = (long)lua_touserdata(L, lua_upvalueindex(3));

	if (idx > len) {
		return 0;
	}

	lua_pushlightuserdata(L, (void*)(1+idx));
	lua_replace(L, lua_upvalueindex(2));

	lua_rawgeti(L, lua_upvalueindex(1), idx);
	if (lua_isnil(L, -1)) {
		luaL_error(L, "Expected array as parameter to 'values', but retrieved a nil value at index '%d'.", idx);
	}
	return 1;
}


namespace lua_api {
	int l_itervalues(lua_State* L) {
		lua_pushvalue(L, 1);
		lua_pushlightuserdata(L, (void*)(1));
		lua_pushlightuserdata(L, (void*)(lua_objlen(L, 1)));
		lua_pushcclosure(L, lapi_values_aux, 3);
		return 1;
	}
	void register_general_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		globals["values"].bind_function(l_itervalues);
		globals["direction"].bind_function(compute_direction);
		globals["distance"].bind_function(distance_between);
	}
}
