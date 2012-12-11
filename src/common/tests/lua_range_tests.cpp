#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/calls.h>
#include <luawrap/testutils.h>

#include "../lua/lua_range.h"
#include "../unittest.h"

static void range_func(const Range& range) {
	UNIT_TEST_ASSERT(range == Range(1,2));
}
static void rangef_func(const RangeF& rangef) {
	UNIT_TEST_ASSERT(rangef == RangeF(.5,1.5));
}

static void lua_range_bind_test() {
	TestLuaState L;
	LuaValue globals = LuaValue::globals(L);
	lua_register_range(L, globals);

	globals.get(L, "range_func") = luawrap::function(L, range_func);
	globals.get(L, "rangef_func") = luawrap::function(L, rangef_func);
	const char* code = "range_func({1,2})\n"
			"rangef_func({.5,1.5})\n";
	lua_assert_valid_dostring(L, code);
}

static void lua_range_call_test() {
	TestLuaState L;
	LuaValue globals = LuaValue::globals(L);
	lua_register_range(L, globals);

	const char* code = "function id(a)\n"
			"return id\n"
			"end\n";

	lua_assert_valid_dostring(L, code);

	globals.get(L, "id").push();
	luawrap::call<void>(L, RangeF());
}

void lua_range_tests() {
	UNIT_TEST(lua_range_bind_test);
	UNIT_TEST(lua_range_call_test);
}
