#include <cstdlib>

#include <UnitTest++.h>

#include <luawrap/testutils.h>
#include <luawrap/functions.h>

//#include "lua/"

struct SimpleType {
	int foo;
	SimpleType(int foo = 2) {
		this->foo = foo;
	}
	bool operator==(const SimpleType& o) {
		return foo == o.foo;
	}

	static void push(lua_State* L, SimpleType t1) {
		lua_pushnumber(L, t1.foo);
	}

	static SimpleType get(lua_State* L, int idx) {
		SimpleType mt(lua_tonumber(L, idx));
		return mt;
	}
	static bool check(lua_State* L, int idx) {
		return lua_isnumber(L, idx);
	}
};

struct MyType2 {
	int foo;
	MyType2(int foo = 2) {
		this->foo = foo;
	}
};

//template<typename T, void (*pushfunc)(lua_State*, const T&), T (*getfunc)(
//		lua_State* L, int idx), bool (*checkfunc)(lua_State*, int)>

template<typename T>
static void luawrap_basic_custom_pushget() {
	TestLuaState L;
	T pushed;
	luawrap::push<T>(L, pushed);
	CHECK(lua_gettop(L) == 1);
	CHECK(luawrap::check<T>(L, -1));

	T got = luawrap::get<T>(L, -1);
	CHECK(pushed == got);
	lua_pop(L, 1);
	L.finish_check();
}

SUITE(luawrap_type_tests) {

	TEST(pushget_simple_type) {
		luawrap::install_type<SimpleType, SimpleType::push, SimpleType::get,
				SimpleType::check>();
		luawrap_basic_custom_pushget<SimpleType>();
	}

}
