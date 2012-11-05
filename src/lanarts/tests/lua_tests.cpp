extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "../lua/LuaValue.h"
#include "tests.h"


void lua_serialize_tests(lua_State* L);

void lua_unit_tests() {
	lua_State* L = lua_open();

	lua_serialize_tests(L);

	lua_close(L);
}
