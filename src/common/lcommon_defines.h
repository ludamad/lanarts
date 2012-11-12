/* lcommon_defines.h:
 *  Defines simple macros that are used throughout the code
 */

#ifndef LCOMMON_DEFINES_H_
#define LCOMMON_DEFINES_H_

#include <cassert>
#include <cstdlib>


// The preferred assert statement to use, for ease of re-implementation
#define LCOMMON_ASSERT(x) assert(x)

// Define a cross-platform function name identifier
#ifdef _MSC_VER
#define FUNCNAME __FUNCSIG__
#else
#ifdef __GNUC__
#define FUNCNAME __PRETTY_FUNCTION__
#else
#define FUNCNAME __func__
#endif
#endif

#endif
