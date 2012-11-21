/*
 * lua_font.h:
 *  Lua bindings for ldraw::Font
 */

#ifndef LUA_FONT_H_
#define LUA_FONT_H_

#include "../Font.h"

struct lua_State;
class LuaValue;

namespace ldraw {

void lua_register_font(lua_State* L, const LuaValue& module);

}

#endif /* LUA_FONT_H_ */
