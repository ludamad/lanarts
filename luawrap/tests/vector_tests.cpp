#include <cstdlib>

#include <UnitTest++.h>

#include <luawrap/testutils.h>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/calls.h>

#include "testassert.h"

template<typename T>
static std::vector<T> vector_func(const std::vector<T>& vec) {
	return vec;
}

template<typename T>
static void lua_generic_vector_test(const char* luastr, const T& t1,
		const T& t2) {
	TestLuaState L;
	LuaValue globals(L, LUA_GLOBALSINDEX);

	std::vector<T> vec;
	vec.push_back(t1);
	vec.push_back(t2);

	globals["vector_func"].bind_function(vector_func<T>);

	std::string code = "return vector_func(";
	code += luastr;
	code += ")\n";
	lua_assert_valid_dostring(L, code.c_str());
	std::vector<T> vec2 = luawrap::get<std::vector<T> >(L, -1);

	unit_test_assert(
			"vectors of type " + std::string(typeid(T).name())
					+ " are not equal.", vec != vec2);
	lua_pop(L, -1);
	L.finish_check();
}

SUITE (luawrap_vector_tests) {
	TEST(vector_basic_tests) {
		lua_generic_vector_test("{1, 2}", 1, 2);
		lua_generic_vector_test("{.5, 1.5}", .5, 1.5);

		std::vector<int> nums1, nums2;
		nums1.push_back(1);
		nums2.push_back(2);

		lua_generic_vector_test("{{1}, {2}}", nums1, nums2);
	}

	TEST(vector_luacall_test) {
		TestLuaState L;

		LuaValue globals(L, LUA_GLOBALSINDEX);

		globals["assert"].bind_function(unit_test_assert);

		std::vector<int> nums;
		nums.push_back(1);
		nums.push_back(2);

		const char* code = "function func(b) "
				"assert(\"List passed differs\", b[1] == 1)"
				"assert(\"List passed differs\", b[2] == 2)"
				"end";

		lua_assert_valid_dostring(L, code);

		globals["func"].push();
		luawrap::call<void>(L, nums);

		L.finish_check();
	}
}
