extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include <common/unittest.h>

#include <common/lua/LuaValue.h>


void lua_unit_tests() {
	lua_State* L = lua_open();

	lua_close(L);
}
