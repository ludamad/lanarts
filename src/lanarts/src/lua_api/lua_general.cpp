/*
 * lua_general.cpp:
 *  General utility functions.
 */
#include <lua.hpp>

#include <luawrap/luawrap.h>

static int lapi_values_aux(lua_State* L) {
	long idx = (long)lua_touserdata(L, lua_upvalueindex(2));
	long len = (long)lua_touserdata(L, lua_upvalueindex(3));

	if (idx > len) {
		return 0;
	}

	lua_pushlightuserdata(L, (void*)(1+idx));
	lua_replace(L, lua_upvalueindex(2));

	lua_rawgeti(L, lua_upvalueindex(1), idx);
	return 1;
}


namespace lua_api {

	int l_itervalues(lua_State* L) {
		lua_pushvalue(L, 1); // array
		lua_pushlightuserdata(L, (void*)1); // initial index
		lua_pushlightuserdata(L, (void*)lua_objlen(L, 1)); // last index
		lua_pushcclosure(L, lapi_values_aux, 3);
		return 1;
	}

	/* Register general utility functions */
	void register_general_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		globals["values"].bind_function(l_itervalues);
	}
}
