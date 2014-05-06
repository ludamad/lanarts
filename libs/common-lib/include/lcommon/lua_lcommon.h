/*
 * lua_lcommon.h:
 *  Lua bindings for lcommon library
 */

#ifndef LUA_LCOMMON_H_
#define LUA_LCOMMON_H_

#include "lua_range.h"
#include "lua_timer.h"
#include "lua_geometry.h"

class LuaValue;
void lua_register_lcommon(lua_State* L);

#endif /* LUA_LCOMMON_H_ */
