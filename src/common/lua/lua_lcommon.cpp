/*
 * lua_lcommon.cpp:
 *  Lua bindings for lcommon library
 */

#include "lua_timer.h"

void lua_register_lcommon(lua_State* L) {
	lua_register_timer(L);
}


