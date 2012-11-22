/*
 * luavalue_tests.cpp:
 *  Tests for LuaValue
 */

#include "../lua/lua_unittest.h"

#include "../lua/lua_geometry.h"
#include "../lua/luacpp.h"
#include "../lua/lua_lcommon.h"

static void luavalue_test_proxy_class() {
	lua_State* L = lua_open();
	SLB::Manager* m = new SLB::Manager;
	m->registerSLB(L);
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_lcommon(L, globals);

	LuaValue value;
	value.table_initialize(L);

	{
		int input = 1337;
		value.get(L, "hello") = input;
		int output = value.get(L, "hello");
		UNIT_TEST_ASSERT(input == output);
	}

	{
		Pos inpos(1, 2);
		value.get(L, "hello") = inpos;
		Pos outpos = value.get(L, "hello");
		UNIT_TEST_ASSERT(inpos == outpos);
	}

	globals.deinitialize(L);
	value.deinitialize(L);

	UNIT_TEST_ASSERT(lua_gettop(L) == 0);
	lua_close(L);
}

static void luavalue_test_value_equality() {
	lua_State* L = lua_open();
	{
		lua_newtable(L);
		LuaValue v1(L, -1), v2(L, -1);
		v1.push(L);
		v2.push(L);
		UNIT_TEST_ASSERT(lua_equal(L, -1, -2));
		lua_pop(L, 3);
	}

	UNIT_TEST_ASSERT(lua_gettop(L) == 0);
	lua_close(L);
}

void luavalue_tests() {
	UNIT_TEST(luavalue_test_proxy_class);
	UNIT_TEST(luavalue_test_value_equality);
}
