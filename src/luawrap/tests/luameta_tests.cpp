/*
 * luameta_tests.cpp:
 *  Test metatable binding helpers
 */

#include <lua.hpp>

#include <UnitTest++.h>

#include <luawrap/luawrap.h>
#include <luawrap/luameta.h>
#include <luawrap/functions.h>
#include <luawrap/members.h>
#include <luawrap/types.h>
#include <luawrap/testutils.h>

#include "testassert.h"

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

static void mymethod(Foo& f, int abc) {
	f.bar = abc;
}

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

	methods.get(L, "mymethod") = luawrap::function(L, mymethod);

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

static void luasetup(TestLuaState& L) {
	LuaValue globals(L, LUA_GLOBALSINDEX);
	install_types();

	globals.get(L, "assert") = luawrap::function(L, unit_test_assert);
	globals.get(L, "foo_unbox1") = luawrap::function(L, foo_unbox1);
	globals.get(L, "foo_unbox2") = luawrap::function(L, foo_unbox2);
	globals.get(L, "foo_unbox3") = luawrap::function(L, foo_unbox3);
	globals.get(L, "foo_unbox4") = luawrap::function(L, foo_unbox4);
	globals.get(L, "foo_unbox5") = luawrap::function(L, foo_unbox5);
	globals.get(L, "foo_box") = luawrap::function(L, foo_box);
}

SUITE (luameta) {

	TEST(foo_bind) { // sanity check
		TestLuaState L;
		luasetup(L);

		lua_assert_valid_dostring(L, " local f = foo_box(1)\n"
				"assert(\"unboxed foo not equivalent\", foo_unbox1(f) == 1)\n"
				"assert(\"unboxed foo not equivalent\", foo_unbox2(f) == 1)\n"
				"assert(\"unboxed foo not equivalent\", foo_unbox3(f) == 1)\n"
				"assert(\"unboxed foo not equivalent\", foo_unbox4(f) == 1)\n"
				"assert(\"unboxed foo not equivalent\", foo_unbox5(f) == 1)\n");
		L.finish_check();
	}

	TEST(luameta_getter) {
		TestLuaState L;
		luasetup(L);

		lua_assert_valid_dostring(L,
				" local f = foo_box(1)\n"
						"assert(\"foo getter for 'bar' not equivalent\", f.bar == 1)\n");
		L.finish_check();
	}

	TEST(luameta_setter) {
		TestLuaState L;
		luasetup(L);

		lua_assert_valid_dostring(L,
				" local f = foo_box(0)\n"
						"f.bar = 1\n"
						"assert(\"foo getter for 'bar' not equivalent\", f.bar == 1)\n");
		L.finish_check();
	}

	TEST(luameta_method) {
		TestLuaState L;
		luasetup(L);

		lua_assert_valid_dostring(L,
				" local f = foo_box(0)\n"
						"assert(\"f.mymethod should not be null\", f.mymethod ~= nil)\n"
						"f:mymethod(1)\n"
						"assert(\"calling f:mymethod(1) should set f.bar to 1\", f.bar == 1)\n");
		L.finish_check();
	}

	TEST(luameta_property_setter) {
		TestLuaState L;
		luasetup(L);

		lua_assert_valid_dostring(L,
				" local f = foo_box(0)\n"
						"f.bar2 = 1\n"
						"assert(\"foo getter for 'bar2' not equivalent\", f.bar2 == 1)\n");
		L.finish_check();
	}

	TEST(luameta_property_setter2) {
		TestLuaState L;
		luasetup(L);

		lua_assert_valid_dostring(L,
				" local f = foo_box(0)\n"
						"f.bar3 = 1\n"
						"assert(\"foo getter for 'bar3' not equivalent\", f.bar3 == 1)\n");
		L.finish_check();
	}

}
