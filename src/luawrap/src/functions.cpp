/*
 * functions.cpp:
 *  Provides ability to push a CPP function.
 */

#include <luawrap/functions.h>

#include <lua.hpp>

namespace luawrap {
	static LuaValue argstring(lua_State* L, int idx) {
		globals(L)["tostring"].push();
		lua_pushvalue(L, idx);
		lua_call(L, 1, 1);

		return LuaValue::pop_value(L);
	}

	namespace _private {
		bool argcheck(lua_State* L, const char* funcname, int idx,
				bool arg_ok) {
			if (arg_ok) {
				return true;
			}

			LuaValue lstring = argstring(L, idx);

			luaL_error(L, "Wrong type for argument %d call to '%s' (was '%s')!", idx,
					funcname, lstring.as<const char*>());
			return false;
		}
	}
}
