/*
 * fatal_error.h:
 *  Represents a generic fatal error.
 *  Due to the complexities of C++ exception handling, no attempt is made to
 *  recover from basic errors. This error is only caught for testing purposes.
 */

#ifndef LCOMMON_FATAL_ERROR_H_
#define LCOMMON_FATAL_ERROR_H_

// Only catch this for testing purposes!
// Eg, to assert that an error _does_ occur
struct __FatalError {
};

void fatal_error();

#endif /* LCOMMON_FATAL_ERROR_H_ */
