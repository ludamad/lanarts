/*
 * luawrap.h:
 *  Implementation of non-template functions in luawrap.h
 */

#include <cstring>

#include <luawrap/luawrap.h>
#include <luawrap/luawraperror.h>

#include "luawrapassert.h"

static std::string format_expression_string(const std::string& str) {
	const char prefix[] = "return ";
	if (strncmp(str.c_str(), prefix, sizeof(prefix)) == 0)
		return str;
	return "return " + str;
}


LuaValue luawrap::eval(lua_State* L, const std::string& code) {
	if (code.empty()) {
		return LuaValue();
	}
	std::string expr = format_expression_string(code);
	int ntop = lua_gettop(L);

	if (luaL_dostring(L, expr.c_str())) {
		std::string failmsg;

		failmsg += "\nWhen running ... \n";
		failmsg += code;
		failmsg += "\n... an error occurred in lua's runtime:\n";
		failmsg += lua_tostring(L, -1);

		luawrap::error(failmsg.c_str());
	}

	LUAWRAP_ASSERT(lua_gettop(L) - ntop == 1);

	LuaValue val(L, -1);
	lua_pop(L, 1);

	return val;
}
