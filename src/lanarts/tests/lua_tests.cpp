extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include <common/unittest.h>

#include <luawrap/LuaValue.h>


void lua_unit_tests() {
	lua_State* L = lua_open();
//TODO

	lua_close(L);
}
