/*
 * lua_timer.cpp:
 *  Registers the Timer class for lua.
 */

#include <SLB/Class.hpp>
#include <SLB/Manager.hpp>

#include "lua_timer.h"

void lua_register_timer(lua_State *L) {
	using namespace SLB;

	Manager* m = Manager::getInstance(L);
	Class<Timer>("Timer", m)
			.constructor()
			.set("start", &Timer::start)
			.set("get_microseconds", &Timer::get_microseconds);
}

