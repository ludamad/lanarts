/*
 * lua_geometry.h:
 *  Lua bindings for geometry structures
 */

#ifndef LUA_GEOMETRY_H_
#define LUA_GEOMETRY_H_

#include "geometry.h"

struct lua_State;
class LuaValue;

// lua state & module is not currently used but passed for future-proofing
void lua_register_geometry(lua_State* L, const LuaValue& module);

#endif /* LUA_GEOMETRY_H_ */
