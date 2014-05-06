/*
 * luawraperror.cpp:
 *  Error functions used on conversion failure and lua runtime failure.
 */

#include <lua.hpp>

#include <luawrap/luawrap.h>

#include <luawrap/luawraperror.h>

namespace luawrap {
	void print_stacktrace(lua_State* L, const std::string& error_message) {
		lua_pushcfunction(L, errorfunc);
		lua_pushlstring(L, error_message.c_str(), error_message.size());
		lua_call(L, 1, 1);
		printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	int errorfunc(lua_State *L) {
		LuaSpecialValue globals = luawrap::globals(L);

		if (!lua_isstring(L, 1)) {
			/* 'message' not a string? */
			if (globals["tostring"].isnil()) {
				lua_pushstring(L, "<Could not convert error to string>");
			} else {
				luawrap::globals(L)["tostring"].push();
				lua_pushvalue(L, 1);
				lua_call(L, 1, 1);
			}
			lua_replace(L, 1);
		}
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

	std::string value_error_string(const std::string& type,
			const std::string& object_path, const std::string& object_repr) {
		std::string error_msg = "Invalid value";
		if (!object_path.empty()) {
			error_msg += " at " + object_path;
		}
		return error_msg + ", expected a " + type + " but got " + object_repr;
	}
}
