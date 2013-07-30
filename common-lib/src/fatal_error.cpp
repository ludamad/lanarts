/*
 * fatal_error.cpp:
 *  Represents a generic fatal error.
 *  Due to the complexities of C++ exception handling, no attempt is made to
 *  recover from basic errors. This error is only caught for testing purposes.
 */

#include <cstdio>

#include "fatal_error.h"

void fatal_error(const char* msg) {
	if (msg) {
		printf("%s", msg);
	}
	fflush(stdout);
	fflush(stderr);
	throw __FatalError();
}
