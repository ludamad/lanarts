#include <SLB/Manager.hpp>
#include <SLB/Script.hpp>

#include "../lua/lua_unittest.h"

#include "../lua/lua_range.h"

static void range_func(const Range& range) {
	UNIT_TEST_ASSERT(range == Range(1,2));
}
static void rangef_func(const RangeF& rangef) {
	UNIT_TEST_ASSERT(rangef == RangeF(.5,1.5));
}

static void lua_range_bind_test() {
	lua_State* L = lua_open();
	luaL_openlibs(L);

	SLB::Manager m;
	m.registerSLB(L);
	{
		m.set("range_func", SLB::FuncCall::create(range_func));
		m.set("rangef_func", SLB::FuncCall::create(rangef_func));
		const char* code = "SLB.range_func({1,2})\n"
				"SLB.rangef_func({.5,1.5})\n";
		lua_assert_valid_dostring(L, code);
	}
	lua_close(L);
}

void lua_range_tests() {
	UNIT_TEST(lua_range_bind_test);
}
