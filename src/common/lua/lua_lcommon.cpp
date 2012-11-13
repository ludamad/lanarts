/*
 * lua_lcommon.cpp:
 *  Lua bindings for lcommon library
 */

#include "lua_timer.h"

#include <SLB/Manager.hpp>
#include <SLB/Table.hpp>

//TODO: Don't expose SLB at all in lua/ folder
//Use it instead as the impl. behind the bindings
//Maybe expose a LuaValue everywhere??
void lua_register_lcommon(lua_State* L, SLB::Table* module) {
	lua_register_timer(L);
	SLB::Manager* m = SLB::Manager::getInstance(L);
//	module->set("Timer", m->getGlobals()->get("Timer"));
}

