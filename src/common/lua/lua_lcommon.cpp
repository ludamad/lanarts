/*
 * lua_lcommon.cpp:
 *  Lua bindings for lcommon library
 */

#include "lua_timer.h"
#include "LuaValue.h"

#include <SLB/Manager.hpp>
#include <SLB/Table.hpp>

void lua_safe_dostring(lua_State* L, const char* code) {
	int stackn = lua_gettop(L);
	if (luaL_dostring(L, code)) {
		std::string failmsg;
		failmsg += "\nWhen running ... \n";
		failmsg += code;
		failmsg += "\n... an error occurred in lua's runtime:\n";
		failmsg += lua_tostring(L, -1);
		printf(failmsg.c_str());
		exit(0);
	}
}

void lua_register_lcommon(lua_State* L, const LuaValue& module) {
	lua_register_timer(L, module);
}

