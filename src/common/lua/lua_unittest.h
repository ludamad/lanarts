#ifndef LUA_UNITTEST_H_
#define LUA_UNITTEST_H_

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

#include "../unittest.h"

void lua_assert_valid_dostring(lua_State* L, const char* code);
void lua_assert_invalid_dostring(lua_State* L, const char* code);

#endif
