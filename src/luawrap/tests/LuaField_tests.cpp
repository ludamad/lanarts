#include <lua.hpp>

#include <luawrap/luawrap.h>
#include <luawrap/testutils.h>
#include <luawrap/LuaField.h>

#include <UnitTest++.h>

SUITE (LuaField) {
	TEST(simple_push) {
		TestLuaState L;

		luawrap::dostring(L, "table = {1, 2, a = 3, b = 4}");
		lua_getfield(L, LUA_GLOBALSINDEX, "table");

		LuaField(L, -1, 1).push();
		CHECK(lua_tointeger(L,-1) == 1);
		lua_pop(L, -1);

		LuaField(L, -1, 2).push();
		CHECK(lua_tointeger(L,-1) == 2);
		lua_pop(L, -1);

		LuaField(L, -1, "a").push();
		CHECK(lua_tointeger(L,-1) == 3);
		lua_pop(L, -1);

		LuaField(L, -1, "b").push();
		CHECK(lua_tointeger(L,-1) == 4);
		lua_pop(L, -1);

		L.finish_check();
	}

	TEST(simple_pop) {
		TestLuaState L;

		luawrap::dostring(L, "table = {1, 2, a = 3, b = 4}");
		lua_getfield(L, LUA_GLOBALSINDEX, "table");

		LuaField(L, -1, 1).push();
		CHECK(lua_tointeger(L,-1) == 1);
		lua_pop(L, -1);

		LuaField(L, -1, 2).push();
		CHECK(lua_tointeger(L,-1) == 2);
		lua_pop(L, -1);

		LuaField(L, -1, "a").push();
		CHECK(lua_tointeger(L,-1) == 3);
		lua_pop(L, -1);

		LuaField(L, -1, "b").push();
		CHECK(lua_tointeger(L,-1) == 4);
		lua_pop(L, -1);

		L.finish_check();
	}
}
