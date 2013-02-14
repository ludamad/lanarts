#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/calls.h>
#include <luawrap/testutils.h>

#include "lua_range.h"
#include "unittest.h"

static void range_func(const Range& range) {
	UNIT_TEST_ASSERT(range == Range(1,2));
}

static void rangef_func(const RangeF& rangef) {
	UNIT_TEST_ASSERT(rangef == RangeF(.5,1.5));
}

SUITE(lua_range_tests) {

	TEST(lua_range_bind_test) {
		TestLuaState L;

		LuaValue globals = luawrap::globals(L);
		lua_register_range(L, globals);

		globals["range_func"].bind_function(range_func);
		globals["rangef_func"].bind_function(rangef_func);
		const char* code = "range_func({1,2})\n"
				"rangef_func({.5,1.5})\n";
		lua_assert_valid_dostring(L, code);

		L.finish_check();
	}

	TEST(lua_range_call_test) {
		TestLuaState L;

		LuaValue globals = luawrap::globals(L);
		lua_register_range(L, globals);

		const char* code = "function id(a)\n"
				"return id\n"
				"end\n";

		lua_assert_valid_dostring(L, code);

		globals["id"].push();
		luawrap::call<void>(L, RangeF());

		L.finish_check();
	}

}
