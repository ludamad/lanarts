/*
 * lua_font.h:
 *  Lua bindings for ldraw::Font
 */

#ifndef LUA_FONT_H_
#define LUA_FONT_H_

#include "../Font.h"

struct lua_State;

void lua_register_font(lua_State* L);

#endif /* LUA_FONT_H_ */
