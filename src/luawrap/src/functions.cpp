/*
 * functions.cpp:
 *  Provides ability to push a CPP function.
 */

#include <luawrap/functions.h>
#include <luawrap/types.h>

#include <lua.hpp>

namespace luawrap {
	static LuaValue argstring(lua_State* L, int idx) {
		globals(L)["tostring"].push();
		lua_pushvalue(L, idx);
		lua_call(L, 1, 1);

		return LuaValue::pop_value(L);
	}

	namespace _private {

		/* Assumes function name is at upvalue 2 */
		bool __argfail(lua_State* L, int idx, const char* expected_type) {
			const char* funcname = lua_tostring(L, lua_upvalueindex(2));
			std::string demangled = demangle_typename(expected_type);
			LuaValue lstring = argstring(L, idx);

			luaL_error(L, "Wrong type for argument %d in call to '%s', expected a %s but got the %s '%s'!",
					idx, funcname, demangled.c_str(), luaL_typename(L, idx),
					lstring.as<const char*>());
			return false;
		}
	}
}
