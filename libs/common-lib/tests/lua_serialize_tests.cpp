#include <lua.hpp>

#include <luawrap/luawrap.h>

#include "luaserialize.h"

#include "unittest.h"

#include "SerializeBuffer.h"

SUITE(lua_serialize_tests) {
	TEST(lua_serialize_number) {
		TestLuaState L;
		SerializeBuffer serializer;

		const double NUM = 1.1;
		lua_pushnumber(L, NUM);
		LuaValue value(L);
		value.pop();

		luaserialize_encode(L, serializer, value);
		LuaValue result;
		luaserialize_decode(L, serializer, result);

		CHECK(result.to_num() == NUM);
	}

	TEST(lua_serialize_table) {
		TestLuaState L;

		SerializeBuffer serializer;
		LuaValue val = LuaValue::newtable(L);
		const double NUM = 1.1;
		val["myfield"] = NUM;
		luaserialize_encode(L, serializer, val);
		LuaValue result;
		luaserialize_decode(L, serializer, result);

		CHECK(result["myfield"].to_num() == NUM);
	}

	TEST(lua_serialize_nil) {
		TestLuaState L;

		SerializeBuffer serializer;
		LuaValue value(L), result(L);
		luaserialize_encode(L, serializer, value);
		luaserialize_decode(L, serializer, result);
		result.push();
		CHECK(lua_isnil(L, -1));

		lua_pop(L, 1);
	}

	TEST(lua_serialize_nested_table) {
		TestLuaState L;

		SerializeBuffer serializer;
		const double VAL = 1.1;

		// Push {{myfield = 1.1}}
		LuaValue mytable = LuaValue::newtable(L);
		mytable[1] = LuaValue::newtable(L);
		mytable[1]["myfield"] = VAL;
		luaserialize_encode(L, serializer, mytable);
		LuaValue result;
		luaserialize_decode(L, serializer, result);
		CHECK(result[1]["myfield"].to_num() == VAL);

	}
//
//	TEST(lua_serialize_with_metatable) {
////		TestLuaState L;
////
////		SerializeBuffer serializer;
////		LuaValue metatable(L), tablepair(L);
////		metatable.newtable(), tablepair.newtable();
////
////		LuaValue value1(L), value2(L);
////		value1.newtable(), value2.newtable();
////
////		value1.set_metatable(metatable), value2.set_metatable(metatable);
////
////		tablepair[1] = value1;
////		tablepair[2] = value2;
////
////		// Sanity check
////		tablepair[1].metatable().push();
////		tablepair[2].metatable().push();
////		CHECK(lua_equal(L, -2, -1));
////		lua_pop(L, 2);
////
////		luaserialize_encode(L, serializer, tablepair);
////		luaserialize_decode(L, serializer, tablepair);
////
////		CHECK(tablepair.metatable().isnil());
////
////		CHECK(!tablepair[1].metatable().isnil());
////		CHECK(!tablepair[2].metatable().isnil());
////		tablepair[1].metatable().push();
////		tablepair[2].metatable().push();
////		// Equality check
////		CHECK(lua_equal(L, -2, -1));
////		lua_pop(L, 2);
////
////		L.finish_check();
//	}
}
