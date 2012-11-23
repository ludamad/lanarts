/*
 * lua_lcommon.cpp:
 *  Lua bindings for lcommon library
 */

#include "lua_timer.h"
#include "LuaValue.h"

#include <SLB/Manager.hpp>
#include <SLB/Table.hpp>

void lua_register_lcommon(lua_State* L, const LuaValue& module) {
	lua_register_timer(L, module);
}

