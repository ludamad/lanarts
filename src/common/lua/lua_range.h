/*
 * lua_range.h:
 *  Lua binding for range
 */

#ifndef LUA_RANGE_H_
#define LUA_RANGE_H_

#include "../Range.h"

#include "luacpp_wrap_numeric.h"

LUACPP_WRAP_AS_NUMARRAY(Range, int);
LUACPP_WRAP_AS_NUMARRAY(RangeF, float);

#endif /* LUA_RANGE_H_ */
