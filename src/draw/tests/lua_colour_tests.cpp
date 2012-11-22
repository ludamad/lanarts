/*
 * lua_colour_tests.cpp:
 *  Test colour bindings in lua
 */

#include <SLB/Manager.hpp>

#include <SLB/FuncCall.hpp>
#include <common/lua/lua_unittest.h>

#include "../lua/lua_colour.h"

static void colour_func(const Colour& range) {
	UNIT_TEST_ASSERT(range == Colour(1,2,3,255));
}

static void lua_colour_bind_test() {
	lua_State* L = lua_open();

	SLB::Manager m;
	m.registerSLB(L);
	m.set("colour_func", SLB::FuncCall::create(colour_func));
	{
		const char* code = "SLB.colour_func({1,2,3,255})\n";
		lua_assert_valid_dostring(L, code);
	}
	{
		const char* code = "SLB.colour_func({1,2,3})\n";
		lua_assert_valid_dostring(L, code);
	}
	{
		const char* code = "SLB.colour_func({1,2})\n";
		lua_assert_invalid_dostring(L, code);
	}
	{
		const char* code = "SLB.colour_func({1,2,3,255,255})\n";
		lua_assert_invalid_dostring(L, code);
	}

	UNIT_TEST_ASSERT(lua_gettop(L) == 0);
	lua_close(L);
}
void lua_colour_tests() {
	UNIT_TEST(lua_colour_bind_test);

}

