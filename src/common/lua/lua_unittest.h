#ifndef LUA_UNITTEST_H_
#define LUA_UNITTEST_H_

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

#include "../unittest.h"

struct TestLuaState {
	lua_State* L;
	TestLuaState() {
		L = lua_open();
	}
	~TestLuaState() {
		UNIT_TEST_ASSERT(lua_gettop(L) == 0);
		lua_close(L);
	}
	operator lua_State*() {
		return L;
	}
};

void lua_assert_valid_dostring(lua_State* L, const char* code);
void lua_assert_invalid_dostring(lua_State* L, const char* code);

#endif
