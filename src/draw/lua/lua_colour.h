/*
 * lua_colour.h:
 *  Bindings for Colour
 */

#ifndef LUA_COLOUR_H_
#define LUA_COLOUR_H_

#include <common/lua/luacpp_wrap_numeric.h>

#include "../Colour.h"

LUACPP_WRAP_AS_NUMARRAY2(Colour, int, 1 /*allowed missing*/,
		255 /*default*/);

class LuaValue;

namespace ldraw {
void lua_register_colour_constants(lua_State *L, const LuaValue& module);
}

#endif /* LUA_COLOUR_H_ */
