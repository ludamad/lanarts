#include <lua.hpp>

#include <luawrap/LuaValue.h>

#include "lua_serialize.h"

#include "unittest.h"

#include "SerializeBuffer.h"

SUITE(lua_serialize_tests) {
	TEST(lua_serialize_number) {
		TestLuaState L;

		SerializeBuffer serializer;
		const double testnumber = 1.1;

		LuaValue value(L), result(L);
		lua_pushnumber(L, testnumber);

		value.pop();
		lua_serialize(serializer, L, value);

		lua_deserialize(serializer, L, result);
		result.push();

		CHECK(lua_isnumber(L, -1));
		CHECK(lua_tonumber(L, -1) == testnumber);

		lua_pop(L, 1);

		L.finish_check();
	}

	TEST(lua_serialize_table) {
		TestLuaState L;

		SerializeBuffer serializer;
		const double testnumber = 1.1;
		const char* fieldname = "myfield";

		// Push {myfield = 1.1}
		lua_newtable(L);
		lua_pushnumber(L, testnumber);
		lua_setfield(L, -2, fieldname);

		{
			//serialize it
			LuaValue value(L), result(L);
			value.pop();
			lua_serialize(serializer, L, value);
			//deserialize it
			lua_deserialize(serializer, L, result);
			result.push();
		}

		CHECK(lua_istable(L, -1));
		lua_getfield(L, -1, fieldname);
		double tablevalue = lua_tonumber(L, -1);
		CHECK(lua_tonumber(L, -1) == testnumber);

		lua_pop(L, 2);
		L.finish_check();
	}

	TEST(lua_serialize_novalue) {
		TestLuaState L;

		SerializeBuffer serializer;
		LuaValue value(L), result(L);
		lua_serialize(serializer, L, value);
		lua_deserialize(serializer, L, result);
		result.push();
		CHECK(lua_isnil(L, -1));

		lua_pop(L, 1);

		L.finish_check();
	}

	TEST(lua_serialize_nested_table) {
		TestLuaState L;

		SerializeBuffer serializer;
		const double testnumber = 1.1;
		const char* fieldname = "myfield";

		// Push {{myfield = 1.1}}
		lua_newtable(L);
		lua_newtable(L);
		lua_pushnumber(L, testnumber);
		lua_setfield(L, -2, fieldname);
		lua_setfield(L, -2, fieldname);

		{
			//serialize it
			LuaValue value(L), result(L);
			value.pop();
			lua_serialize(serializer, L, value);
			lua_deserialize(serializer, L, result);
			result.push();
		}

		CHECK(lua_istable(L, -1));
		lua_getfield(L, -1, fieldname);
		CHECK(lua_istable(L, -1));
		lua_getfield(L, -1, fieldname);
		double tablevalue = lua_tonumber(L, -1);
		CHECK(lua_tonumber(L, -1) == testnumber);

		lua_pop(L, 3);

		L.finish_check();
	}

	TEST(lua_serialize_with_metatable) {
		TestLuaState L;

		SerializeBuffer serializer;
		LuaValue metatable(L), tablepair(L);
		metatable.newtable(), tablepair.newtable();

		LuaValue value1(L), value2(L);
		value1.newtable(), value2.newtable();

		value1.set_metatable(metatable), value2.set_metatable(metatable);

		tablepair[1] = value1;
		tablepair[2] = value2;

		lua_serialize(serializer, L, tablepair);
		lua_deserialize(serializer, L, tablepair);

		CHECK(!tablepair[1].metatable().isnil());
		CHECK(!tablepair[2].metatable().isnil());
		tablepair[1].metatable().push();
		tablepair[2].metatable().push();
		// Equality check
		CHECK(lua_equal(L, -2, -1));
		lua_pop(L, 2);

		L.finish_check();
	}
}
