#include <typeinfo>

#include <SLB/Manager.hpp>
#include <SLB/Script.hpp>

#include <SLB/FuncCall.hpp>
#include <SLB/LuaCall.hpp>

#include "../lua/lua_unittest.h"

#include "../lua/lua_geometry.h"
#include "../lua/lua_vector.h"

template<typename T>
static std::vector<T> vector_func(const std::vector<T>& vec) {
	return vec;
}

template<typename T>
static void lua_generic_vector_test(const char* luastr, const T& t1,
		const T& t2) {
	lua_State* L = lua_open();

	SLB::Manager m;
	m.registerSLB(L);

	std::vector<T> vec;
	vec.push_back(t1);
	vec.push_back(t2);
	m.set("vector_func", SLB::FuncCall::create(vector_func<T>));

	std::string code = "return SLB.vector_func(";
	code += luastr;
	code += ")\n";
	lua_assert_valid_dostring(L, code.c_str());
	std::vector<T> vec2 = SLB::get<std::vector<T> >(L, -1);
	unit_test_assert(
			"vectors of type " + std::string(typeid(T).name())
					+ " are not equal.", vec == vec2);
	lua_pop(L, -1);

	std::string fail_code = "return SLB.vector_func({})";

	lua_close(L);
}

static void lua_vector_test() {
	lua_generic_vector_test("{1, 2}", 1, 2);
	lua_generic_vector_test("{.5, 1.5}", .5, 1.5);

	std::vector<int> nums1, nums2;
	nums1.push_back(1);
	nums2.push_back(2);

	lua_generic_vector_test("{{1}, {2}}", nums1, nums2);
	lua_generic_vector_test("{{1,2}, {3,4}}", Pos(1, 2), Pos(3, 4));
}

static void lua_vector_luacall_test() {
	lua_State* L = lua_open();
	{
		SLB::Manager m;
		m.registerSLB(L);
		m.set("assert", SLB::FuncCall::create(unit_test_assert));

		std::vector<int> nums;
		nums.push_back(1);
		nums.push_back(2);

		const char* code = "function func(b) "
				"SLB.assert(\"List passed differs\", b[1] == 1)"
				"SLB.assert(\"List passed differs\", b[2] == 2)"
				"end";

		lua_assert_valid_dostring(L, code);

		SLB::LuaCall<void(const std::vector<int>&)> call(L, "func");
		call(nums);
	}
	lua_close(L);
}

void lua_vector_tests() {
	UNIT_TEST(lua_vector_test);
	UNIT_TEST(lua_vector_luacall_test);
}
