/*
 * types.cpp:
 *  Install types, implemented via a template-powered generic callback dispatch
 *  system.
 */

#include <lua.hpp>

#include <luawrap/types.h>
#include <luawrap/luawraperror.h>

namespace luawrap {
	namespace _private {

		Callbacks::Callbacks(push_t pushcallback, get_t getcallback,
				check_t checkcallback) {
			this->pushcallback = pushcallback;
			this->getcallback = getcallback;
			this->checkcallback = checkcallback;
		}

		Callbacks::Callbacks() {
			this->pushcallback = NULL;
			this->getcallback = NULL;
			this->checkcallback = NULL;
		}

		void install_callbacks(Callbacks& cb, push_t pushcallback,
				get_t getcallback, check_t checkcallback) {

#ifndef LUAWRAP_ALLOW_REDEFINE
			if (cb.checkcallback || cb.getcallback || cb.pushcallback) {
				if (cb.checkcallback != checkcallback
						&& cb.getcallback != getcallback
						&& cb.pushcallback != pushcallback) {
					error(
							"Attempt to redefine type that has previously been installed!\n"
									"Policy is set to exit, if you would like this to be allowed "
									"configure luawrap with LUAWRAP_ALLOW_REDEFINE");
				}
			}
#endif

			cb.checkcallback = checkcallback;
			cb.pushcallback = pushcallback;
			cb.getcallback = getcallback;
		}

		bool check_is_luametatype(lua_State* L, int idx,
				luameta_initializer meta) {
			lua_getmetatable(L, idx);
			lua_pushlightuserdata(L, (void*) meta);
			lua_gettable(L, LUA_REGISTRYINDEX);
			bool eq = lua_rawequal(L, -2, -1);
			lua_pop(L, 2);
			return eq;
		}

	}
}

#ifdef __GNUC__
#include <cxxabi.h>

/* Attempt to make the output of typeinfo readable for nicer errors, if possible. */
std::string luawrap::_private::demangle_typename(const char* name) {
	int status;
	const char* demangled = abi::__cxa_demangle(name, 0, 0, &status);
	if (status != 0) {
		return name;
	}
	std::string newname(demangled);
	return newname;
}

#else

/* Attempt to make the output of typeinfo readable for nicer errors, if possible. */
std::string luawrap::_private::demangle_typename(const char* name) {
	return name;
}
#endif
