/*
 * LuaAction.h:
 *  Represents a game action, with a prerequisite for use.
 */

#ifndef LUAACTION_H_
#define LUAACTION_H_

#include "../lua/LuaValue.h"

struct LuaAction {
	std::string success_message;
	std::string failure_message; // If does not pass pre-req function
	LuaValue action_func, prereq_func;

	void init(lua_State* L) {
		action_func.initialize(L);
		prereq_func.initialize(L);
	}
	LuaAction(const LuaValue& action_func = LuaValue()) :
			action_func(action_func) {
	}
};

#endif /* LUAACTION_H_ */
