#include <lua.hpp>

#include <luawrap/LuaValue.h>

#include "lua_serialize.h"

#include "unittest.h"

#include "SerializeBuffer.h"

SUITE(lua_serialize_tests) {
	TEST(lua_serialize_number) {
		TestLuaState L;

		SerializeBuffer serializer = SerializeBuffer::plain_buffer();
		const double testnumber = 1.1;

		LuaValue value(L), result(L);
		lua_pushnumber(L, testnumber);

		value.pop();
		lua_serialize(serializer, L, value);

		lua_deserialize(serializer, L, result);
		result.push();

		UNIT_TEST_ASSERT(lua_isnumber(L, -1));
		UNIT_TEST_ASSERT(lua_tonumber(L, -1) == testnumber);

		lua_pop(L, 1);

		L.finish_check();
	}

	TEST(lua_serialize_table) {
		TestLuaState L;

		SerializeBuffer serializer = SerializeBuffer::plain_buffer();
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

		UNIT_TEST_ASSERT(lua_istable(L, -1));
		lua_getfield(L, -1, fieldname);
		double tablevalue = lua_tonumber(L, -1);
		UNIT_TEST_ASSERT(lua_tonumber(L, -1) == testnumber);

		lua_pop(L, 2);
		L.finish_check();
	}

	TEST(lua_serialize_novalue) {
		TestLuaState L;

		SerializeBuffer serializer = SerializeBuffer::plain_buffer();
		LuaValue value(L), result(L);
		lua_serialize(serializer, L, value);
		lua_deserialize(serializer, L, result);
		result.push();
		UNIT_TEST_ASSERT(lua_isnil(L, -1));

		lua_pop(L, 1);

		L.finish_check();
	}

	TEST(lua_serialize_nested_table) {
		TestLuaState L;

		SerializeBuffer serializer = SerializeBuffer::plain_buffer();
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

		UNIT_TEST_ASSERT(lua_istable(L, -1));
		lua_getfield(L, -1, fieldname);
		UNIT_TEST_ASSERT(lua_istable(L, -1));
		lua_getfield(L, -1, fieldname);
		double tablevalue = lua_tonumber(L, -1);
		UNIT_TEST_ASSERT(lua_tonumber(L, -1) == testnumber);

		lua_pop(L, 3);

		L.finish_check();
	}
}
