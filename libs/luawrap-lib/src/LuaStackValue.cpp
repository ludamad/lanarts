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

#include <luawrap/luawrap.h>

#include "luawrapassert.h"

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
	LUAWRAP_ASSERT(L);
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

LuaField LuaStackValue::operator [](std::string& key) const {
	return LuaField(L, idx, key.c_str());
}

LuaField LuaStackValue::operator [](const char* key) const {
	return LuaField(L, idx, key);
}

LuaField LuaStackValue::operator [](int index) const {
	return LuaField(L, idx, index);
}

bool LuaStackValue::has(const char* key) const {
	return !(*this)[key].isnil();
}

void LuaStackValue::newtable() const {
	lua_newtable(L);
	pop();
}

bool LuaStackValue::isnil() const {
	return lua_isnil(L, idx);
}

void LuaStackValue::pop() const {
	lua_replace(L, idx);
}

LuaStackValue::operator LuaField() const {
	return LuaField(L, idx);
}

LuaValue LuaStackValue::metatable() const {
	push();
	if (!lua_getmetatable(L, -1)) {
		lua_pushnil(L);
	}
	LuaValue table(L, -1);
	lua_pop(L, 2);
	return table;
}


void LuaStackValue::set_nil() const {
	lua_pushnil(L);
	pop();
}

static void error_and_pop(lua_State* L, const std::string& expected_type) {
	const char* repr = lua_tostring(L, -1);
	std::string obj_repr = repr ? repr : "nil";
	lua_pop(L, 1);
	luawrap::conversion_error(expected_type, "", obj_repr);
}


void* LuaStackValue::to_userdata() const {
	lua_State* L = luastate();
	push();
	void* userdata = lua_touserdata(L, -1);
	if (!userdata) {
		error_and_pop(L, "userdata");
	}
	lua_pop(L, 1);
	return userdata;
}

double LuaStackValue::to_num() const {
	push();
	if (!lua_isnumber(L, -1)) {
		error_and_pop(L, "double");
	}
	double num = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return num;
}


int LuaStackValue::to_int() const {
	push();

	//TODO: Evaluate whether full integer checking is necessary
	double num = lua_tonumber(L, -1);
	int integer = (int) num;
	if (!lua_isnumber(L, -1) || num != integer) {
		error_and_pop(L, "integer");
	}
	lua_pop(L, 1);
	return integer;
}

bool LuaStackValue::to_bool() const {
	push();
	if (!lua_isboolean(L, -1)) {
		error_and_pop(L, "boolean");
	}
	bool boolean = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return boolean;
}

const char* LuaStackValue::to_str() const {
	push();
	if (!lua_isstring(L, -1)) {
		error_and_pop(L, "string");
	}
	const char* str = lua_tostring(L, -1);
	lua_pop(L, 1);
	return str;
}

int LuaStackValue::objlen() const {
	push();
	if (!lua_istable(L, -1)) {
		error_and_pop(L, "table");
	}
	int len = lua_objlen(L, -1);
	lua_pop(L, 1);
	return len;
}

void LuaStackValue::set(int pos) {
	lua_pushvalue(L, pos);
	pop();
}

void LuaStackValue::set_metatable(const LuaField& metatable) const {
	push();
	metatable.push();
	lua_setmetatable(L, -2);
	lua_pop(L, 1);
}
