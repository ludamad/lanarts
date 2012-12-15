/*
 * luaexpr.cpp:
 *  Function for calling a function and storing the result.
 */

#include <cstdio>
#include <luawrap/LuaValue.h>
#include <lcommon/perf_timer.h>

#include "lanarts_defines.h"
#include "luaexpr.h"

#include <lua.hpp>

void luavalue_call_and_store(lua_State* L, LuaValue& eval) {
	perf_timer_begin(FUNCNAME);

	if (eval.empty()) {
		perf_timer_end(FUNCNAME);
		return;
	}

	eval.push();

	if (lua_pcall(L, 0, 1, 0 /*no errfunc*/)) {
		printf("Error while evaluating lua expression:\n%s\n",
				lua_tostring(L, -1));
		exit(0);
	}

	eval.pop();
	perf_timer_end(FUNCNAME);
}
