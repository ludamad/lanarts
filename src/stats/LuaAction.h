/*
 * LuaAction.h:
 *  Represents a game action, with a prerequisite for use.
 */

#ifndef LUAACTION_H_
#define LUAACTION_H_

#include <lcommon/LuaLazyValue.h>

struct LuaAction {
	std::string success_message;
	std::string failure_message; // If does not pass pre-req function
	LuaValue action_func, prereq_func;
};

#endif /* LUAACTION_H_ */
