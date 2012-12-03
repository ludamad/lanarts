/*
 * luameta_tests.cpp:
 *  Test metatable binding helpers
 */

#include "../unittest.h"
#include "../lua/lua_unittest.h"

#include "../lua/luacpp.h"
#include "../lua/luacpp_wrap.h"
#include "../lua/luacpp_wrap_function.h"
#include "../lua/luacpp_metatable.h"

static LuaValue foo_newmetatable(lua_State* L);

struct Foo {
	int bar;

	static Foo& get(lua_State* L, int idx) {
		return *(Foo*)lua_touserdata(L, idx);
	}
	static void push(lua_State* L, const Foo& foo) {
		Foo* f = (Foo*)lua_newuserdata(L, sizeof(Foo));
		*f = foo;
		luameta_pushcached(L, foo_newmetatable);
		lua_setmetatable(L, -2);
	}
	static bool check(lua_State* L, int idx) {
		return lua_isuserdata(L, idx);
	}

	int& get_bar() {
		return bar;
	}

	void set_bar(int b) {
		bar = b;
	}

	void set_bar2(const int& b) {
		bar = b;
	}

};

//LUACPP_TYPE_WRAP(Foo*, Foo::push, Foo::get, Foo::check);
LUACPP_GENERAL_WRAP(, const Foo&, Foo&, Foo&, Foo::push, Foo::get, Foo::check);
LUACPP_GENERAL_WRAP(, const Foo&, Foo, Foo, Foo::push, Foo::get, Foo::check);
//LUACPP_TYPE_WRAP_IMPL(Foo*);

static LuaValue foo_newmetatable(lua_State* L) {
	LuaValue metatable = luameta_new(L, "Foo");
	luameta_member<Foo, int, &Foo::bar>(L, metatable, "bar");
	luameta_member<Foo, int, &Foo::get_bar>(L, metatable, "bar2");
	luameta_setter_member<Foo, int, &Foo::set_bar>(L, metatable, "bar3");
	luameta_getter_member<Foo, int, &Foo::bar>(L, metatable, "bar3");
	return metatable;
}

static int foo_unbox(Foo& f) {
	return f.bar;
}
static Foo foo_box(int bar) {
	Foo f = { bar };
	return f;
}

static lua_State* luasetup() {
	lua_State* L = lua_open();
	LuaValue globals(L, LUA_GLOBALSINDEX);

	luaL_openlibs(L);

	globals.get(L, "assert") = luavalue_from_function(L, unit_test_assert);
	globals.get(L, "foo_unbox") = luavalue_from_function(L, foo_unbox);
	globals.get(L, "foo_box") = luavalue_from_function(L, foo_box);

	return L;
}
static void foo_bind_test() { // sanity check
	lua_State* L = luasetup();

	lua_assert_valid_dostring(L, " local f = foo_box(1)\n"
			"assert(\"unboxed foo not equivalent\", foo_unbox(f) == 1)\n");

	UNIT_TEST_ASSERT(lua_gettop(L) == 0);
	lua_close(L);
}
static void test_luameta_getter() {
	lua_State* L = luasetup();

	lua_assert_valid_dostring(L, " local f = foo_box(1)\n"
			"assert(\"foo getter for 'bar' not equivalent\", f.bar == 1)\n");

	lua_close(L);
}
static void test_luameta_setter() {
	lua_State* L = luasetup();

	lua_assert_valid_dostring(L, " local f = foo_box(0)\n"
			"f.bar = 1\n"
			"assert(\"foo getter for 'bar' not equivalent\", f.bar == 1)\n");

	lua_close(L);
}

static void test_luameta_property_setter() {
	lua_State* L = luasetup();

	lua_assert_valid_dostring(L, " local f = foo_box(0)\n"
			"f.bar2 = 1\n"
			"assert(\"foo getter for 'bar2' not equivalent\", f.bar2 == 1)\n");

	lua_close(L);
}

static void test_luameta_property_setter2() {
	lua_State* L = luasetup();

	lua_assert_valid_dostring(L, " local f = foo_box(0)\n"
			"f.bar3 = 1\n"
			"assert(\"foo getter for 'bar3' not equivalent\", f.bar3 == 1)\n");

	lua_close(L);
}

void luameta_tests() {
	UNIT_TEST(foo_bind_test);
	UNIT_TEST(test_luameta_getter);
	UNIT_TEST(test_luameta_setter);
	UNIT_TEST(test_luameta_property_setter);
	UNIT_TEST(test_luameta_property_setter2);
}
