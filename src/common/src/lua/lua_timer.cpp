/*
 * lua_timer.cpp:
 *  Registers the Timer class for lua.
 */

#include <luawrap/LuaValue.h>
#include <luawrap/luameta.h>
#include <luawrap/functions.h>
#include <luawrap/types.h>

#include "perf_timer.h"
#include "lua_timer.h"

static void start(Timer* timer) {
	timer->start();
}

static unsigned long get_microseconds(Timer* timer) {
	return timer->get_microseconds();
}

LuaValue lua_timermetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "Timer");
	LuaValue methods = luameta_methods(meta);

	methods["start"].bind_function(start);
	methods["get_microseconds"].bind_function(get_microseconds);

	luameta_gc<Timer>(meta);

	return meta;
}

static Timer newtimer() {
	return Timer();
}

void lua_register_timer(lua_State *L, const LuaValue& module) {
	luawrap::install_userdata_type<Timer, &lua_timermetatable>();

	module["timer_create"].bind_function(newtimer);

	module["perf_timer_begin"].bind_function(perf_timer_begin);
	module["perf_timer_end"].bind_function(perf_timer_end);
	module["perf_timer_clear"].bind_function(perf_timer_clear);
	module["perf_print_results"].bind_function(perf_print_results);
}

