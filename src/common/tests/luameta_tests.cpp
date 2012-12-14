/*
 * luameta_tests.cpp:
 *  Test metatable binding helpers
 */

#include <lua.hpp>

#include <luawrap/luawrap.h>
#include <luawrap/luameta.h>
#include <luawrap/functions.h>
#include <luawrap/members.h>
#include <luawrap/types.h>
#include <luawrap/testutils.h>

#include "unittest.h"

struct Foo {
	int bar;

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

LuaValue foo_newmetatable(lua_State* L) {
	LuaValue metatable = luameta_new(L, "Foo");
	LuaValue methods = luameta_methods(L, metatable);
	LuaValue getters = luameta_getters(L, metatable);
	LuaValue setters = luameta_setters(L, metatable);

	getters.get(L, "bar") = &luawrap::getter<Foo, int, &Foo::bar>;
	setters.get(L, "bar") = &luawrap::setter<Foo, int, &Foo::bar>;

	getters.get(L, "bar2") = &luawrap::getter<Foo, int, &Foo::get_bar>;
	setters.get(L, "bar2") = &luawrap::setter<Foo, int, &Foo::get_bar>;

	getters.get(L, "bar3") = &luawrap::getter<Foo, int, &Foo::bar>;
	setters.get(L, "bar3") = &luawrap::setter<Foo, int, &Foo::set_bar>;

	return metatable;
}

static void install_types() {
	luawrap::install_userdata_type<Foo, foo_newmetatable>();
}

static int foo_unbox1(Foo& f) {
	return f.bar;
}

static int foo_unbox2(const Foo& f) {
	return f.bar;
}
static int foo_unbox3(Foo* f) {
	return f->bar;
}
static int foo_unbox4(Foo f) {
	return f.bar;
}
static int foo_unbox5(const Foo* f) {
	return f->bar;
}

static Foo foo_box(int bar) {
	Foo f = { bar };
	return f;
}

static lua_State* luasetup() {
	lua_State* L = lua_open();
	LuaValue globals(L, LUA_GLOBALSINDEX);


	globals.get(L, "assert") = luawrap::function(L, unit_test_assert);
	globals.get(L, "foo_unbox1") = luawrap::function(L, foo_unbox1);
	globals.get(L, "foo_unbox2") = luawrap::function(L, foo_unbox2);
	globals.get(L, "foo_unbox3") = luawrap::function(L, foo_unbox3);
	globals.get(L, "foo_unbox4") = luawrap::function(L, foo_unbox4);
	globals.get(L, "foo_unbox5") = luawrap::function(L, foo_unbox5);
	globals.get(L, "foo_box") = luawrap::function(L, foo_box);

	return L;
}

static void foo_bind_test() { // sanity check
	lua_State* L = luasetup();

	lua_assert_valid_dostring(L, " local f = foo_box(1)\n"
			"assert(\"unboxed foo not equivalent\", foo_unbox1(f) == 1)\n"
			"assert(\"unboxed foo not equivalent\", foo_unbox2(f) == 1)\n"
			"assert(\"unboxed foo not equivalent\", foo_unbox3(f) == 1)\n"
			"assert(\"unboxed foo not equivalent\", foo_unbox4(f) == 1)\n"
			"assert(\"unboxed foo not equivalent\", foo_unbox5(f) == 1)\n");

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
	install_types();
	UNIT_TEST(foo_bind_test);
	UNIT_TEST(test_luameta_getter);
	UNIT_TEST(test_luameta_setter);
	UNIT_TEST(test_luameta_property_setter);
	UNIT_TEST(test_luameta_property_setter2);
}
