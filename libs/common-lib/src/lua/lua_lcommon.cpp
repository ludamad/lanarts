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
#include "lua_mtwist.h"

static void lua_extend(lua_State* L, lua_CFunction func, const char* module_name) {
        lua_getfield(L, LUA_GLOBALSINDEX, "package");
        lua_getfield(L, -1, "preload");
        lua_pushcfunction(L, func);
        lua_setfield(L, -2, module_name);
}

void lua_register_lcommon(lua_State* L) {
	LuaValue globals = luawrap::globals(L);
	lua_register_timer(L, globals);

	lua_register_geometry(L, globals);
	lua_register_range(L, globals);
	lua_extend(L, luaopen_mtwist, "mtwist");
}
