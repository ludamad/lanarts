/*
 * luavalue_tests.cpp:
 *  Tests for LuaValue
 */

#include <luawrap/luawrap.h>
#include <luawrap/testutils.h>

#include <UnitTest++.h>

SUITE (LuaValue) {
	TEST(luavalue_test_get) {
		TestLuaState L;

		LuaValue globals = luawrap::globals(L);

		LuaValue value(L);
		value.newtable();

		{
			int input = 1337;
			value["hello"] = input;
			int output = value["hello"].as<int>();
			CHECK_EQUAL(input, output);
		}

		{
			std::string input = "test";
			value["hello"] = input;
			std::string output = value["hello"].to_str();
			CHECK(input == output);
		}

		globals.clear();
		value.clear();
		L.finish_check();
	}

	TEST(luavalue_test_value_equality) {
		TestLuaState L;

		lua_newtable(L);
		LuaValue v1(L, -1), v2(L, -1);
		v1.push();
		v2.push();
		CHECK(lua_equal(L, -1, -2));
		lua_pop(L, 3);
		L.finish_check();
	}
}
