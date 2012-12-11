/*
 * LuaAction.h:
 *  Represents a game action, with a prerequisite for use.
 */

#ifndef LUAACTION_H_
#define LUAACTION_H_

#include <luawrap/LuaValue.h>

#include "../lua/luaexpr.h"

struct LuaAction {
	std::string success_message;
	std::string failure_message; // If does not pass pre-req function
	LuaValue action_func, prereq_func;

	void init(lua_State* L) {
		luavalue_call_and_store(L, action_func);
		luavalue_call_and_store(L, prereq_func);
	}
	LuaAction(const LuaValue& action_func = LuaValue()) :
			action_func(action_func) {
	}
};

#endif /* LUAACTION_H_ */
