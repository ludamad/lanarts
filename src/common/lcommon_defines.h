/* lcommon_defines.h:
 *  Defines simple structures that are used throughout the lanarts components
 */

#ifndef LCOMMON_DEFINES_H_
#define LCOMMON_DEFINES_H_

#include <cassert>
#include <cstdlib>

// If this file is not included there may be issues with Microsoft's compiler
#ifdef _MSC_VER
#define snprintf _snprintf
#define round(x) floor((x)+0.5f)
#endif

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
