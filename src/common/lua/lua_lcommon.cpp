/*
 * lua_lcommon.cpp:
 *  Lua bindings for lcommon library
 */

#include "lua_timer.h"
#include "LuaValue.h"

#include <SLB/Manager.hpp>
#include <SLB/Table.hpp>

//TODO: Don't expose SLB at all in lua/ folder ?
//Use it instead as the impl. behind the bindings
//Maybe expose a LuaValue everywhere??
void lua_register_lcommon(lua_State* L, LuaValue& module) {
	lua_register_timer(L, module);
}

