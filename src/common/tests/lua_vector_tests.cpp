#include <typeinfo>lua_lua_serialize_testsserialize_tests

#include <SLB/Manager.hpp>
#include <SLB/Script.hpp>

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

void lua_vector_tests() {
	UNIT_TEST(lua_vector_test);
}
