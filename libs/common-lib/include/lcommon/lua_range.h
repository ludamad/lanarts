/*
 * lua_range.h:
 *  Lua binding for range
 */

#ifndef LUA_RANGE_H_
#define LUA_RANGE_H_

#include "Range.h"

struct lua_State;
class LuaValue;

// lua state & module is not currently used but passed for future-proofing
void lua_register_range(lua_State* L, const LuaValue& module);

#endif /* LUA_RANGE_H_ */
