#include "lua_unittest.h"

#include <string>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

void lua_assert_valid_dostring(lua_State* L, const char* code) {
	int stackn = lua_gettop(L);
	if (luaL_dostring(L, code)) {
		std::string failmsg;
		failmsg += "\nWhen running ... \n";
		failmsg += code;
		failmsg += "\n... an error occurred in lua's runtime:\n";
		failmsg += lua_tostring(L, -1);
		unit_test_fail(failmsg);
	}
}

void lua_assert_invalid_dostring(lua_State* L, const char* code) {
	int stackn = lua_gettop(L);
	if (!luaL_dostring(L, code)) {
		std::string failmsg;
		failmsg += "Expected error when running ... \n";
		failmsg += code;
		failmsg += "\n... but code completed!\n";
		unit_test_fail(failmsg);
	}
}
