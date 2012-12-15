/*
 * lua_timer.cpp:
 *  Registers the Timer class for lua.
 */

#include <luawrap/LuaValue.h>
#include <luawrap/luameta.h>
#include <luawrap/functions.h>
#include <luawrap/types.h>

#include "lua_timer.h"

static void start(Timer* timer) {
	timer->start();
}

static unsigned long get_microseconds(Timer* timer) {
	return timer->get_microseconds();
}

LuaValue lua_timermetatable(lua_State* L) {
	LuaValue val = luameta_new(L, "Timer");
	LuaValue methods = luameta_methods(L, val);

	methods["start"].bind_function(start);
	methods["get_microseconds"].bind_function(get_microseconds);

	return val;
}

void lua_register_timer(lua_State *L, const LuaValue& module) {
	luawrap::install_userdata_type<Timer, &lua_timermetatable>();
}

