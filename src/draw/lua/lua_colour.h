/*
 * lua_colour.h:
 *  Bindings for Colour
 */

#ifndef LUA_COLOUR_H_
#define LUA_COLOUR_H_

#include <common/lua/slb_numvectortype.h>

#include "../Colour.h"

SLB_NUMERIC_VECTOR_WITH_PARAMS(Colour, int, 1 /*allowed missing*/, 255 /*default*/);

class LuaValue;

void lua_register_colour_constants(lua_State *L, LuaValue& module);

#endif /* LUA_COLOUR_H_ */
