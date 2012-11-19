/*
 * lua_range.h:
 *  Lua binding for range
 */

#ifndef LUA_RANGE_H_
#define LUA_RANGE_H_

#include "../range.h"

#include "slb_numvectortype.h"

SLB_NUMERIC_VECTOR(Range, int);
SLB_NUMERIC_VECTOR(RangeF, float);

#endif /* LUA_RANGE_H_ */
