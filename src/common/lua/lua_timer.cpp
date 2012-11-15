/*
 * lua_timer.cpp:
 *  Registers the Timer class for lua.
 */

#include <SLB/Class.hpp>
#include <SLB/Manager.hpp>

#include "LuaValue.h"
#include "lua_timer.h"
#include "luacpp.h"

void lua_register_timer(lua_State *L, LuaValue& module) {
	const char CLASSNAME[] = "Timer";
	using namespace SLB;

	Manager* m = Manager::getInstance(L);
	Class<Timer>(CLASSNAME, m).constructor().set("start", &Timer::start).set(
			"get_microseconds", &Timer::get_microseconds);
	m->getGlobals()->get(CLASSNAME)->push(L);
	module.get(L, CLASSNAME).pop();
}

