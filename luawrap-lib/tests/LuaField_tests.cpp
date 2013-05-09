#include <lua.hpp>

#include <luawrap/luawrap.h>
#include <luawrap/testutils.h>
#include <luawrap/LuaField.h>

#include <luawrap/luawraperror.h>

#include <UnitTest++.h>

SUITE (LuaField) {
	TEST(simple_push) {
		TestLuaState L;

		luawrap::dostring(L, "table = {1, 2, a = 3, b = 4}");
		lua_getfield(L, LUA_GLOBALSINDEX, "table");

		LuaField(L, -1, 1).push();
		CHECK(lua_tointeger(L,-1) == 1);
		lua_pop(L, 1);

		LuaField(L, -1, 2).push();
		CHECK(lua_tointeger(L,-1) == 2);
		lua_pop(L, 1);

		LuaField(L, -1, "a").push();
		CHECK(lua_tointeger(L,-1) == 3);
		lua_pop(L, 1);

		LuaField(L, -1, "b").push();
		CHECK(lua_tointeger(L,-1) == 4);
		lua_pop(L, 1);

		/* pop global */
		lua_pop(L, 1);
		L.finish_check();
	}

	TEST(simple_pop) {
		TestLuaState L;

		{
			const int KEY = 1;
			LuaField field(L, LUA_GLOBALSINDEX, KEY);
			lua_pushnumber(L, 1.1);
			field.pop();

			lua_pushinteger(L, KEY);
			lua_gettable(L, LUA_GLOBALSINDEX);
			CHECK(lua_tonumber(L, -1) == 1.1);
			lua_pop(L, 1);
		}

		{
			const char* KEY = "a";
			LuaField field(L, LUA_GLOBALSINDEX, KEY);
			lua_pushnumber(L, 1.1);
			field.pop();

			lua_getfield(L, LUA_GLOBALSINDEX, KEY);
			CHECK(lua_tonumber(L, -1) == 1.1);
			lua_pop(L, 1);
		}

		L.finish_check();
	}

	TEST(chained_push) {
		TestLuaState L;
		std::string teststring = "teststring";

		{
			luawrap::dostring(L,
					"table = { { [2] = 'teststring' } }");
			lua_getfield(L, LUA_GLOBALSINDEX, "table");

			LuaField(L, -1, 1)[2].push();
			CHECK( lua_tostring(L, -1) == teststring);

			lua_pop(L, 2);
		}

		{
			luawrap::dostring(L,
					"table = { { [2] = { a = { b = 'teststring'} } } }");
			lua_getfield(L, LUA_GLOBALSINDEX, "table");

			LuaField(L, -1, 1)[2]["a"]["b"].push();
			CHECK( lua_tostring(L, -1) == teststring);

			lua_pop(L, 2);
		}
		L.finish_check();
	}

	TEST(chained_error) {
		TestLuaState L;
		std::string teststring = "teststring";

		int preop = lua_gettop(L);
		try {
			luawrap::dostring(L,
					"table = { { { a = { b = {} } }  } }");
			lua_getfield(L, LUA_GLOBALSINDEX, "table");

			LuaField(L, -1, 1)[1]["a"]["b"]["c"][1].push();
			CHECK(false);
		} catch (const luawrap::ValueError& lve) {
			CHECK( lve.object_path() == "[1][1][\"a\"][\"b\"][\"c\"]" );
			lua_settop(L, preop);
		}

		L.finish_check();
	}
}
