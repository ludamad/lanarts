/*
 * lua_range.h:
 *  Lua binding for range
 */

#ifndef LUA_RANGE_H_
#define LUA_RANGE_H_

#include "../range.h"

#include "slb_valuetype.h"

SLB_WRAP_VALUE_TYPE_AS_NUMARR(Range, int);
SLB_WRAP_VALUE_TYPE_AS_NUMARR(RangeF, float);

#endif /* LUA_RANGE_H_ */
