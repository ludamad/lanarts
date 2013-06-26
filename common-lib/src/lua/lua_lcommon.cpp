/*
 * lua_lcommon.cpp:
 *  Lua bindings for lcommon library
 */


#include <lua.hpp>
#include <luawrap/LuaValue.h>

#include "fatal_error.h"

#include "lua_timer.h"
#include "lua_geometry.h"
#include "lua_range.h"

void lua_register_lcommon(lua_State* L) {
	LuaValue globals = luawrap::globals(L);
	lua_register_timer(L, globals);

	lua_register_geometry(L, globals);
	lua_register_range(L, globals);
}
