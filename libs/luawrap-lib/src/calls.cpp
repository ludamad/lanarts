/*
 * calls.cpp:
 *  Call lua functions by passing appropriate C++ types.
 */

#include <lua.hpp>
#include <luawrap/luawraperror.h>

namespace luawrap {
	namespace _private {
		// Throws/aborts if false
		void luacall_return_check(bool check_result) {
			if (!check_result) {
				luawrap::error(
						"Return value could not be converted when calling luawrap::call, exitting...\n");
			}
		}

		void luacall_with_traceback(lua_State* L, int nargs) {
			lua_pushcfunction(L, luawrap::errorfunc);

			lua_insert(L, -2 - nargs); // swap places with pushed function
			if (lua_pcall(L, nargs, 1, -2 - nargs) != 0) {
				const char* errstr = lua_tostring(L, -1);
				error(errstr ? errstr : "<Invalid error string>");
			}
			lua_replace(L, -2);
		}
	}
}
