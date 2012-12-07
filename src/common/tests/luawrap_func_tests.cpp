#include <cstdlib>

#include "../lua/lua_unittest.h"

#include "../luawrap/functions.h"

template<typename T>
static T identity1(T a) {
	return a;
}
template<typename T>
static T identity2(const T& a) {
	return a;
}

template<typename T, typename V>
static void test_num_func(T func, V num) {
	lua_State* L = lua_open();

	luawrap::push_cppfunction(L, "testfunc", func);
	UNIT_TEST_ASSERT(lua_gettop(L) == 1);

	lua_pushnumber(L, num);
	lua_call(L, 1, 1);

	UNIT_TEST_ASSERT(lua_tonumber(L, -1) == num);

	lua_pop(L, 1);
}

static void luawrap_num_func() {
#define SUBTEST( x, n) \
	printf("calling " #x "\n"); \
	test_num_func(x, 2)

	SUBTEST(identity1<int>, 2);
	SUBTEST(identity2<int>, 2);
	//If identity2 works once, it should work the other times

	SUBTEST(identity1<char>, 255);
	SUBTEST(identity1<short>, 3);
	SUBTEST(identity1<long>, 4);
	SUBTEST(identity1<float>, 1.1);
	SUBTEST(identity1<double>, 1.1);
}

void luawrap_function_tests() {
	UNIT_TEST(luawrap_num_func);
}
