/*
 * lua_array_util.h:
 *  Utilities for dealing with array-like lua tables.
 */

#include <luawrap/LuaValue.h>
#include <luawrap/LuaField.h>

void luaarray_append(const LuaField& array, const LuaField& value);
// Clearing is unfortunately an O(N) operation
void luaarray_clear(const LuaField& array);
