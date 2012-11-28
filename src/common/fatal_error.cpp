/*
 * fatal_error.cpp:
 *  Represents a generic fatal error.
 *  Due to the complexities of C++ exception handling, no attempt is made to
 *  recover from basic errors. This error is only caught for testing purposes.
 */

#include "fatal_error.h"

void fatal_error() {
	throw __FatalError();
}

