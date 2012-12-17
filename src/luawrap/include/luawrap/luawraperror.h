/*
 * luawraperror.h:
 *  Error functions used on conversion failure and lua runtime failure.
 */

#ifndef LUAWRAP_LUAWRAPERROR_H_
#define LUAWRAP_LUAWRAPERROR_H_

#include <luawrap/config.h>

#ifdef LUAWRAP_USE_EXCEPTIONS

#include <stdexcept>

struct lua_State;

namespace luawrap {
	class Exception: public std::runtime_error {
	public:
		Exception(const std::string& msg) :
				std::runtime_error(msg) {

		}
	};

	inline void error(const char* msg) {
		throw Exception(msg);
	}

	int errorfunc(lua_State *L);
}

#else

#include <cstdio>
#include <cstdlib>

namespace luawrap {
	void error(const char* msg) {
		fprintf(stderr, msg);
		fflush(stderr);
		abort();
	}
}

#endif

#endif /* LUAWRAP_LUAWRAPERROR_H_ */
