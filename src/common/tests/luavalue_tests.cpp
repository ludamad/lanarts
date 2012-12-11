/*
 * luavalue_tests.cpp:
 *  Tests for LuaValue
 */

#include <luawrap/luawrap.h>
#include <luawrap/testutils.h>

#include "../lua/lua_lcommon.h"

#include "../unittest.h"
#include "../geometry.h"

static void luavalue_test_get() {
	TestLuaState L;

	LuaValue globals = LuaValue::globals(L);
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
}

static void luavalue_test_value_equality() {
	TestLuaState L;

	lua_newtable(L);
	LuaValue v1(L, -1), v2(L, -1);
	v1.push(L);
	v2.push(L);
	UNIT_TEST_ASSERT(lua_equal(L, -1, -2));
	lua_pop(L, 3);
}

#include "../perf_timer.h"
static void lua_perf_test() {
	TestLuaState L;

	perf_timer_begin("lua_binding");
	for (int i = 0; i < 10000; i++) {
		lua_State* L = lua_open();
		lua_register_lcommon(L, LuaValue(L, LUA_GLOBALSINDEX));

		lua_close(L);
	}
	perf_timer_end("lua_binding");

	perf_timer_begin("lua_newtable");
	for (int i = 0; i < 10000; i++) {
		lua_newtable(L);
		lua_pop(L, 1);
	}
	perf_timer_end("lua_newtable");

	perf_timer_begin("lua_newtable_with_luavalue");
	for (int i = 0; i < 10000; i++) {
		lua_newtable(L);
		LuaValue val(L, -1);
		lua_pop(L, 1);
	}
	perf_timer_end("lua_newtable_with_luavalue");

	perf_timer_begin("lua_ref_withtable");
	for (int i = 0; i < 10000; i++) {
		lua_newtable(L);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		luaL_unref(L, LUA_REGISTRYINDEX, ref);
	}
	perf_timer_end("lua_ref_withtable");

	perf_timer_begin("lua_dostring_with_luavalue");
	for (int i = 0; i < 10000; i++) {
		LuaValue val = luawrap::eval(L, "function(a) return {} end");
	}
	perf_timer_end("lua_dostring_with_luavalue");

	perf_timer_begin("lua_store_string_with_luavalue");
	for (int i = 0; i < 10000; i++) {
		LuaValue val = luawrap::eval(L, "function(a) return {} end");
	}
	perf_timer_end("lua_store_string_with_luavalue");

	perf_timer_begin("lua_get_set_pos");
	LuaValue value(L, LUA_GLOBALSINDEX);
	for (int i = 0; i < 10000; i++) {
		Pos inpos(1, 2);
		value.get(L, "hello") = inpos;
		Pos outpos = value.get(L, "hello");
	}
	perf_timer_end("lua_get_set_pos");

	perf_timer_begin("lua_get_pos");
	for (int i = 0; i < 10000; i++) {
		Pos outpos = value.get(L, "hello");
	}
	perf_timer_end("lua_get_pos");

	perf_print_results();
}

void luavalue_tests() {
	UNIT_TEST(luavalue_test_get);
	UNIT_TEST(luavalue_test_value_equality);
	UNIT_TEST(lua_perf_test);
}
