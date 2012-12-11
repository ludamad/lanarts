/*
 * lua_colour_tests.cpp:
 *  Test colour bindings in lua
 */

#include <lua.hpp>

#include <common/unittest.h>

#include <luawrap/testutils.h>
#include <luawrap/LuaValue.h>
#include <luawrap/functions.h>

#include "../Colour.h"

#include "../lua/lua_ldraw.h"

static void colour_func(const Colour& range) {
	UNIT_TEST_ASSERT(range == Colour(1,2,3,255));
}

static void lua_colour_bind_test() {
	TestLuaState L;

	LuaValue globals = LuaValue::globals(L);

	ldraw::lua_register_ldraw(L, globals);

	globals.get(L, "colour_func") = luawrap::function(L, colour_func);
	{
		const char* code = "colour_func({1,2,3,255})\n";
		lua_assert_valid_dostring(L, code);
	}
	{
		const char* code = "colour_func({1,2,3})\n";
		lua_assert_valid_dostring(L, code);
	}
	{
		const char* code = "colour_func({1,2})\n";
		lua_assert_invalid_dostring(L, code);
	}
	{
		const char* code = "colour_func({1,2,3,255,255})\n";
		lua_assert_invalid_dostring(L, code);
	}
	L.finish_check();
}
void lua_colour_tests() {
	UNIT_TEST(lua_colour_bind_test);

}

