/*
 * testutils.h:
 *  Utilites for testing luawrap-using code.
 */

#include <string>
#include <stdexcept>

#include <lua.hpp>

#include <luawrap/testutils.h>

#include "luawrapassert.h"

TestLuaState::TestLuaState() {
	L = lua_open();
}
TestLuaState::~TestLuaState() {
	if (!std::uncaught_exception()) {
		finish_check();
	}
	lua_close(L);
}

void lua_assert_valid_dostring(lua_State* L, const char* code) {
	int stackn = lua_gettop(L);
	if (luaL_dostring(L, code)) {
		std::string failmsg;
		failmsg += "\nWhen running ... \n";
		failmsg += code;
		failmsg += "\n... an error occurred in lua's runtime:\n";
		failmsg += lua_tostring(L, -1);
		throw std::runtime_error(failmsg);
	}
	lua_pop(L, lua_gettop(L) - stackn);
}

void lua_assert_invalid_dostring(lua_State* L, const char* code) {
	int stackn = lua_gettop(L);
	if (!luaL_dostring(L, code)) {
		std::string failmsg;
		failmsg += "Expected error when running ... \n";
		failmsg += code;
		failmsg += "\n... but code completed!\n";
		throw std::runtime_error(failmsg);
	} else {
		lua_pop(L, lua_gettop(L) - stackn);
	}
}

void TestLuaState::finish_check() const {
	if (lua_gettop(L) != 0) {
		char entry_buff[32];
		sprintf(entry_buff, "%d", lua_gettop(L));
		throw std::runtime_error(
				std::string("Failed assertion after test, lua_gettop(L) == 0, was ") + entry_buff);
	}
}

