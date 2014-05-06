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
	LuaValue methods = luameta_constants(metatable);
	LuaValue getters = luameta_getters(metatable);
	LuaValue setters = luameta_setters(metatable);

	luawrap::bind_getter( getters["bar"], &Foo::bar);
	luawrap::bind_setter( setters["bar"], &Foo::bar);

	getters["bar2"] = &luawrap::getter<Foo, int, &Foo::get_bar>;
	setters["bar2"] = &luawrap::setter<Foo, int, &Foo::get_bar>;

	luawrap::bind_getter( getters["bar3"], &Foo::bar);
	setters["bar3"] = &luawrap::setter<Foo, int, &Foo::set_bar>;

	methods["mymethod"].bind_function(mymethod);

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

	globals["assert"].bind_function(unit_test_assert);
	globals["foo_unbox1"].bind_function(foo_unbox1);
	globals["foo_unbox2"].bind_function(foo_unbox2);
	globals["foo_unbox3"].bind_function(foo_unbox3);
	globals["foo_unbox4"].bind_function(foo_unbox4);
	globals["foo_unbox5"].bind_function(foo_unbox5);
	globals["foo_box"].bind_function(foo_box);
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
