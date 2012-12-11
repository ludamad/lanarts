/*
 * luawraperror.h:
 *  Exception thrown when conversions fail.
 */

#ifndef LUAWRAP_LUAWRAPERROR_H_
#define LUAWRAP_LUAWRAPERROR_H_

#include <luawrap/config.h>

#ifdef LUAWRAP_USE_EXCEPTIONS

#include <stdexcept>

namespace luawrap {
	class Exception: public std::runtime_error {
	public:
		Exception(const std::string& msg) :
				std::runtime_error(msg) {

		}
	};
}

namespace luawrap {
	inline void error(const char* msg) {
		throw new Exception(msg);
	}
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
