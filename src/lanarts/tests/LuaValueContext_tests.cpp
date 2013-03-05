#include <lcommon/unittest.h>
#include <luawrap/luawrap.h>

#include "lua_api/LuaValueContext.h"

SUITE(LuaValueContext_tests) {
	TEST(test_index_error_message) {
		TestLuaState L;
		luawrap::dostring(L, "table = { a = { b = { } } }");

		LuaValue value = luawrap::globals(L)["table"];
		LuaValueContext context(value);
		try {
			context["a"];
			context["a"]["b"];
			context["a"]["b"]["c"];
		} catch (const LuaValueContextIndexError& lvcie) {
			std::string error = lvcie.what();
			CHECK( error.find("[\"a\"][\"b\"][\"c\"]") != std::string::npos );
		}

		L.finish_check();
	}
}
