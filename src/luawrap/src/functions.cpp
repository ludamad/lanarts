/*
 * functions.cpp:
 *  Provides ability to push a CPP function.
 */

#include <luawrap/functions.h>

#include <lua.hpp>

namespace luawrap {
	namespace _private {
		bool argcheck(lua_State* L, const char* funcname, int argn,
				bool arg_ok) {
			if (arg_ok) {
				return true;
			}
			luaL_error(L, "Wrong type for argument %d call to '%s'!", argn,
					funcname);
			return false;
		}
	}
}
