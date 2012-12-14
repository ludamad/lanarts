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

using namespace _luawrap_private;

namespace _luawrap_private {

	void _LuaStackField::push() const {
	}

	void _LuaStackField::pop() const {
	}

	void _LuaStackField::operator =(const LuaValue& value) {
	}

	void _LuaStackField::operator =(const char* value) {
	}

	void _LuaStackField::operator =(lua_CFunction func) {
	}

	void _LuaStackField::operator =(const _LuaField& field) {
	}

	void _LuaStackField::operator =(const _LuaStackField& field) {
	}
}

LuaStackValue::LuaStackValue(lua_State* L, int idx) :
		L(L), idx(idx) {
}

LuaStackValue::LuaStackValue() :
		L(NULL) {
	/* idx is undefined and should not be used */
}

void LuaStackValue::push(lua_State* L) const {
	lua_pushvalue(L, idx);
}

bool LuaStackValue::empty() const {
	return L == NULL;
}

LuaStackValue luawrap::globals(lua_State* L) {
	return LuaStackValue(L, LUA_GLOBALSINDEX);
}

