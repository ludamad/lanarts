/*
 * lua_colour.h:
 *  Bindings for Colour
 */

#ifndef LUA_COLOUR_H_
#define LUA_COLOUR_H_

#include "Colour.h"

class LuaValue;

namespace ldraw {
void lua_register_colour(lua_State *L, const LuaValue& module);
}

#endif /* LUA_COLOUR_H_ */
