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

		LuaValue globals = LuaValue::globals(L);

		LuaValue value;
		value.table_initialize(L);

		{
			int input = 1337;
			value.get(L, "hello") = input;
			int output = value.get(L, "hello");
			CHECK_EQUAL(input, output);
		}

		{
			std::string input = "test";
			value.get(L, "hello") = input;
			std::string output = value.get(L, "hello");
			CHECK(input == output);
		}

		globals.deinitialize(L);
		value.deinitialize(L);
		L.finish_check();
	}

	TEST(luavalue_test_value_equality) {
		TestLuaState L;

		lua_newtable(L);
		LuaValue v1(L, -1), v2(L, -1);
		v1.push(L);
		v2.push(L);
		CHECK(lua_equal(L, -1, -2));
		lua_pop(L, 3);
		L.finish_check();
	}
}
