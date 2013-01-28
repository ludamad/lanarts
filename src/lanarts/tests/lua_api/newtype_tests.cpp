#include <lua.hpp>

#include <lcommon/unittest.h>

#include <luawrap/LuaValue.h>
#include <luawrap/luawrap.h>
#include <luawrap/testutils.h>
#include <lua_api/lua_newapi.h>

static void newtype_basic() {
	TestLuaState L;
	lua_api::register_general_api(L);

	LuaValue newtype = luawrap::globals(L)["newtype"];
	UNIT_TEST_ASSERT(!newtype.isnil());

	const char code[] = "bar = newtype().create()";
	lua_assert_valid_dostring(L, code);

	LuaValue bar = luawrap::globals(L)["bar"];
	UNIT_TEST_ASSERT(!bar.isnil());

	L.finish_check();
}

static void newtype_init() {
	TestLuaState L;
	lua_api::register_general_api(L);

	const char code[] = "Foo = newtype() \n"
			"function Foo:init(member) self.member = member end \n"
			"bar = Foo.create('test')";
	lua_assert_valid_dostring(L, code);

	LuaValue bar = luawrap::globals(L)["bar"];
	UNIT_TEST_ASSERT(!bar.isnil());

	LuaValue member = bar["member"];

	UNIT_TEST_ASSERT(!member.isnil());
	UNIT_TEST_ASSERT(member.as<std::string>() == "test");

	L.finish_check();
}

static void newtype_getter() {
	TestLuaState L;
	lua_api::register_general_api(L);

	const char code[] = "Foo = newtype() \n"
			"function Foo:init(member) self.member = member end \n"
			"function Foo.get:getter() return self.member end \n"
			"got = Foo.create('test').getter";

	lua_assert_valid_dostring(L, code);

	LuaValue got = luawrap::globals(L)["got"];

	UNIT_TEST_ASSERT(!got.isnil());
	UNIT_TEST_ASSERT(got.as<std::string>() == "test");

	L.finish_check();
}

static void newtype_setter() {
	TestLuaState L;
	lua_api::register_general_api(L);

	const char code[] = "Foo = newtype() \n"
			"function Foo.set:setter(value) self.member = value end \n"
			"bar = Foo.create() \n"
			"bar.setter = 'test' \n"
			"set = bar.member";

	lua_assert_valid_dostring(L, code);

	LuaValue set = luawrap::globals(L)["set"];

	UNIT_TEST_ASSERT(!set.isnil());
	UNIT_TEST_ASSERT(set.as<std::string>() == "test");

	L.finish_check();
}

void lua_api_tests() {
	UNIT_TEST(newtype_basic);
	UNIT_TEST(newtype_init);
	UNIT_TEST(newtype_getter);
	UNIT_TEST(newtype_setter);
}
