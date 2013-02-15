#include <lua.hpp>

#include <lcommon/unittest.h>

#include <luawrap/LuaValue.h>
#include <luawrap/luawrap.h>
#include <luawrap/testutils.h>
#include <lua_api/lua_newapi.h>

SUITE(lua_api_tests) {

	TEST(newtype_basic) {
		TestLuaState L;
		lua_api::register_general_api(L);

		LuaValue newtype = luawrap::globals(L)["newtype"];
		CHECK(!newtype.isnil());

		const char code[] = "bar = newtype().create()";
		lua_assert_valid_dostring(L, code);

		LuaValue bar = luawrap::globals(L)["bar"];
		CHECK(!bar.isnil());

		L.finish_check();
	}

	TEST(newtype_init) {
		TestLuaState L;
		lua_api::register_general_api(L);

		const char code[] = "Foo = newtype() \n"
				"function Foo:init(member) self.member = member end \n"
				"bar = Foo.create('test')";
		lua_assert_valid_dostring(L, code);

		LuaValue bar = luawrap::globals(L)["bar"];
		CHECK(!bar.isnil());

		LuaValue member = bar["member"];

		CHECK(!member.isnil());
		CHECK(member.as<std::string>() == "test");

		L.finish_check();
	}

	TEST(newtype_getter) {
		TestLuaState L;
		lua_api::register_general_api(L);

		const char code[] = "Foo = newtype() \n"
				"function Foo:init(member) self.member = member end \n"
				"function Foo.get:getter() return self.member end \n"
				"got = Foo.create('test').getter";

		lua_assert_valid_dostring(L, code);

		LuaValue got = luawrap::globals(L)["got"];

		CHECK(!got.isnil());
		CHECK(got.as<std::string>() == "test");

		L.finish_check();
	}

	TEST(newtype_setter) {
		TestLuaState L;
		lua_api::register_general_api(L);

		const char code[] = "Foo = newtype() \n"
				"function Foo.set:setter(value) self.member = value end \n"
				"bar = Foo.create() \n"
				"bar.setter = 'test' \n"
				"set = bar.member";

		lua_assert_valid_dostring(L, code);

		LuaValue set = luawrap::globals(L)["set"];

		CHECK(!set.isnil());
		CHECK(set.as<std::string>() == "test");

		L.finish_check();
	}

}
