/*
 * lua_timer.cpp:
 *  Registers the Timer class for lua.
 */

#include <SLB/Class.hpp>
#include <SLB/Manager.hpp>

#include "slb.h"
#include "LuaValue.h"
#include "lua_timer.h"
#include "luacpp.h"

void lua_register_timer(lua_State *L, const LuaValue& module) {
	const char CLASSNAME[] = "Timer";
	using namespace SLB;

	Manager* m = getOrCreateManager(L);
	Class<Timer>(CLASSNAME, m).constructor().set("start", &Timer::start).set(
			"get_microseconds", &Timer::get_microseconds);
	m->getGlobals()->get(CLASSNAME)->push(L);
	module.get(L, CLASSNAME).pop();
}

