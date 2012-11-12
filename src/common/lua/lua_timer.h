/*
 * lua_timer.h:
 *  Registers the Timer class for lua.
 */

#ifndef LUA_TIMER_H_
#define LUA_TIMER_H_

#include "../Timer.h"

struct lua_State;

void lua_register_timer(lua_State* L);

#endif /* LUA_TIMER_H_ */
