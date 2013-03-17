#include <lua.hpp>

#include <luawrap/luawrap.h>
#include <luawrap/testutils.h>

#include <UnitTest++.h>

SUITE (LuaStackValue) {
	// Also tests table logic:
	TEST(luastackvalue_global_table_test) {
		TestLuaState L;

		LuaSpecialValue globals = luawrap::globals(L);

		globals["abc"] = 1;

		lua_getglobal(L, "abc");
		int abc = luawrap::pop<int>(L);

		CHECK_EQUAL(1, abc);

		abc = globals["abc"].to_int();
		CHECK_EQUAL(1, abc);

		L.finish_check();
	}
}
