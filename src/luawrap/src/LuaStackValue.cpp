/**
 * LuaStackValue.cpp:
 *  Represents a value on the lua stack.
 *  This is represented by an index and care must be taken for it not to
 *  become invalid during its use. Can also be used to represent special values
 *  like LUA_GLOBALSINDEX. A typedef, LuaSpecialValue
 *
 *  Generally it should not be used as a class member.
 *
 *  Provides a proxy class for useful table operations, eg val["table_key"] = somevalue;
 *  A number of conversions are supported, however if you want 'somevalue' to be some
 *  arbitrary type you must also include luawrap.h.
 *
 *  Another convenient form, val["table_key].function(func) is provided. This relies on
 *  functions.h.
 */

#include <lua.hpp>

#include <luawrap/LuaStackValue.h>
#include <luawrap/LuaValue.h>

using namespace _luawrap_private;

namespace _luawrap_private {

	void _LuaStackField::push() const {
		lua_getfield(value.luastate(), value.index(), key);
	}

	void _LuaStackField::pop() const {
		lua_setfield(value.luastate(), value.index(), key);
	}

	void _LuaStackField::operator =(const LuaValue& value) {
		value.push();
		pop();
	}

	void _LuaStackField::operator =(const char* str) {
		lua_pushstring(value.luastate(), str);
		pop();
	}

	void _LuaStackField::operator =(lua_CFunction func) {
		lua_pushcfunction(value.luastate(), func);
		pop();
	}

	void _LuaStackField::operator =(const _LuaField& field) {
		field.push();
		pop();
	}


	void _LuaStackField::operator =(const _LuaStackField& field) {
		field.push();
		pop();
	}

	bool _LuaStackField::isnil() const {
		push();
		bool nil = lua_isnil(value.luastate(), -1);
		lua_pop(L, 1);
		return nil;
	}

	void _LuaStackField::bind_function(lua_CFunction func) {
		lua_pushcfunction(value.luastate(), func);
		pop();
	}

	LuaValue _LuaStackField::ensure_table() const {
		lua_State* L = value.luastate();
		push();

		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			lua_newtable(L);
			lua_pushvalue(L, -1);
			pop();
		}

		return LuaValue::pop_value(L);
	}
}

LuaStackValue::LuaStackValue(lua_State* L, int idx) :
		L(L), idx(idx) {
	if (idx > LUA_REGISTRYINDEX + 100 && idx < 1) {
		this->idx += lua_gettop(L) + 1; // Convert relative negative position to 'absolute' stack position
	}
}

LuaStackValue::LuaStackValue() :
		L(NULL) {
	/* idx is undefined and should not be used */
}

void LuaStackValue::push() const {
	lua_pushvalue(L, idx);
}

bool LuaStackValue::empty() const {
	return L == NULL;
}

LuaSpecialValue luawrap::globals(lua_State* L) {
	return LuaSpecialValue(L, LUA_GLOBALSINDEX);
}

LuaSpecialValue luawrap::registry(lua_State* L) {
	return LuaSpecialValue(L, LUA_REGISTRYINDEX);
}

int LuaStackValue::objlen() const {
	push();
	int len = lua_objlen(L, -1);
	lua_pop(L, 1);
	return len;
}

LuaValue LuaStackValue::operator [](int idx) const {
	push();
	lua_rawgeti(L, -1, idx);
	LuaValue ret = LuaValue::pop_value(L);
	lua_pop(L, 1);
	return ret;
}
