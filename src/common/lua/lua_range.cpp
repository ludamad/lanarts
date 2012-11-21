/*
 * lua_range.cpp:
 *  Lua binding for range
 */

#include "lua_range.h"

LUACPP_WRAP_AS_NUMARRAY_IMPL(Range, int);
LUACPP_WRAP_AS_NUMARRAY_IMPL(RangeF, float);
