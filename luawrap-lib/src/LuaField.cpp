/**
 * LuaField.cpp:
 *	PLEASE READ BEFORE USING.
 *
 *  Represents a value inside another lua object.
 *  This is represented by an index chain of sorts and *MUST* be stored on the stack.
 *  There is some trade-off of usability and performance here, but generally it is very fast.
 *
 *  It should NOT be used as a class member. It primarily exists as a proxy convenience class.
 *
 *  You have been warned.
 */

#include <lua.hpp>

#include <cstring>

#include <luawrap/LuaField.h>
#include <luawrap/luawraperror.h>
#include <luawrap/LuaValue.h>
#include <luawrap/LuaStackValue.h>

#include "luawrapassert.h"

/*****************************************************************************
 *                          Constructors                                     *
 *****************************************************************************/
enum {
	STRING_INDEX = 0, INTEGER_INDEX = 1, NO_INDEX = 2
};

enum {
	FIELD_PARENT = 0, REGISTRY_PARENT = 1, STACK_PARENT = 2
};

LuaField::LuaField(lua_State* L, void* parent, const char* index) {
	Index ind;
	Parent par;
	par.registry = parent;
	ind.string = index;
	init(L, STRING_INDEX, REGISTRY_PARENT, ind, par);
}

LuaField::LuaField(lua_State* L, void* parent, int index) {
	Index ind;
	Parent par;
	par.registry = parent;
	ind.integer = index;
	init(L, INTEGER_INDEX, REGISTRY_PARENT, ind, par);
}

LuaField::LuaField(lua_State* L, void* parent) {
	Index ind;
	Parent par;
	par.registry = parent;
	init(L, NO_INDEX, REGISTRY_PARENT, ind, par);
}


LuaField::LuaField(const LuaField* parent, const char* index) {
	Index ind;
	Parent par;
	par.field = parent;
	ind.string = index;
	init(parent->L, STRING_INDEX, FIELD_PARENT, ind, par);
}

LuaField::LuaField(const LuaField* parent, int index) {
	Index ind;
	Parent par;
	par.field = parent;
	ind.integer = index;
	init(parent->L, INTEGER_INDEX, FIELD_PARENT, ind, par);
}

static void adjust_stack_index(lua_State* L, int& stackidx) {
	if (stackidx > LUA_REGISTRYINDEX + 100 && stackidx < 1) {
		stackidx += lua_gettop(L) + 1; // Convert relative negative position to 'absolute' stack position
	}
}
LuaField::LuaField(lua_State* L, int stackidx, const char* index) {
	adjust_stack_index(L, stackidx);
	Index ind;
	Parent par;
	par.stack_index = stackidx;
	ind.string = index;
	init(L, STRING_INDEX, STACK_PARENT, ind, par);
}

LuaField::LuaField(lua_State* L, int stackidx, int index) {
	adjust_stack_index(L, stackidx);
	Index ind;
	Parent par;
	par.stack_index = stackidx;
	ind.integer = index;
	init(L, INTEGER_INDEX, STACK_PARENT, ind, par);
}

LuaField::LuaField(lua_State* L, int stackidx) {
	adjust_stack_index(L, stackidx);
	Index ind;
	Parent par;
	par.stack_index = stackidx;
	init(L, NO_INDEX, STACK_PARENT, ind, par);
}

bool LuaField::has(const char* key) const {
	return !(*this)[key].isnil();
}

const LuaField& LuaField::assert_not_nil() const {
	push();
	if (!lua_isnil(L, -1)) {
		lua_pop(L, 1);
	} else {
		error_and_pop("non-nil");
	}
	return *this;
}

LuaValue LuaField::metatable() const {
	push();
	if (!lua_getmetatable(L, -1)) {
		lua_pushnil(L);
	}
	LuaValue table(L, -1);
	lua_pop(L, 2);
	return table;
}

void LuaField::set_metatable(const LuaField& metatable) const {
	lua_State* L = luastate();
	push();
	metatable.push();
	lua_setmetatable(L, -2);
	lua_pop(L, 1);
}

void LuaField::set(int pos) {
	lua_pushvalue(L, pos);
	pop();
}

void LuaField::init(lua_State* L, char index_type, char parent_type,
		Index index, Parent parent) {
	this->L = L;
	this->_index_type = index_type;
	this->_parent_type = parent_type;
	this->_index_type = index_type;
	this->_index = index;
	this->_parent = parent;

}

/*****************************************************************************
 *                         Index path creation                               *
 *****************************************************************************/

std::string LuaField::index_path() const {
	std::string str;
	index_path(str);
	return str;
}

void LuaField::index_path(std::string& str) const {
	if (_parent_type == FIELD_PARENT) {
		_parent.field->index_path(str);
	}

	if (_index_type == STRING_INDEX) {
		str += "[\"";
		str += _index.string;
		str += "\"]";
	} else if (_index_type == INTEGER_INDEX){
		char buff[32];
		sprintf(buff, "[%d]", _index.integer);
		str += buff;
	}
}

/*****************************************************************************
 *                         Utility methods                                   *
 *****************************************************************************/

void LuaField::push() const {
	push_parent();
	if (_index_type == STRING_INDEX) {
		lua_getfield(L, -1, _index.string);
		lua_replace(L, -2);
	} else if (_index_type == INTEGER_INDEX){
		lua_pushinteger(L, _index.integer);
		lua_gettable(L, -2);
		lua_replace(L, -2);
	} else {// == NO_INDEX
		/* Nothing, return parent */
	}
}

void LuaField::raw_push() const {
	push_parent();
	if (_index_type == STRING_INDEX) {
		lua_pushstring(L, _index.string);
	} else if (_index_type == INTEGER_INDEX){
		lua_pushinteger(L, _index.integer);
	} else {// == NO_INDEX
		/* Nothing, return parent */
		return;
	}
	lua_rawget(L, -2);
	lua_replace(L, -2);
}

void LuaField::handle_nil_parent() const {
	std::string idxpath = "";
	if (_parent_type == FIELD_PARENT) {
		_parent.field->index_path(idxpath);
	}
	luawrap::nil_error(idxpath);
}

void LuaField::push_parent() const {
	if (_parent_type != FIELD_PARENT) {
		/* Parent is not field */
		if (_parent_type == REGISTRY_PARENT) {
			lua_pushlightuserdata(L, _parent.registry);
			lua_rawget(L, LUA_REGISTRYINDEX);
		} else {
			lua_pushvalue(L, _parent.stack_index);
		}
	} else {
		LUAWRAP_ASSERT(_parent.field->luastate() == L);
		_parent.field->push();
	}

	if (lua_isnil(L, -1) && _index_type != NO_INDEX) {
		lua_pop(L, 1);
		handle_nil_parent();
	}
}

void LuaField::pop() const {
	if (_index_type == NO_INDEX) {
		luawrap::error("Cannot mutate LuaField that has no index!");
	}
	push_parent();

	if (_index_type == STRING_INDEX) {
		lua_pushstring(L, _index.string);
	} else {
		lua_pushinteger(L, _index.integer);
	}

	/* Value to pop */
	lua_pushvalue(L, -3);
	/* Parent */
	lua_settable(L, -3);
	/* Pop parent and value to pop*/
	lua_pop(L, 2);
}

void LuaField::raw_pop() const {
	if (_index_type == NO_INDEX) {
		luawrap::error("Cannot mutate LuaField that has no index!");
	}
	push_parent();

	if (_index_type == STRING_INDEX) {
		lua_pushstring(L, _index.string);
	} else {
		lua_pushinteger(L, _index.integer);
	}

	/* Value to pop */
	lua_pushvalue(L, -3);
	/* Parent */
	lua_rawset(L, -3);
	/* Pop parent and value to pop*/
	lua_pop(L, 2);
}

/*****************************************************************************
 *                         Container methods                                 *
 *****************************************************************************/

LuaField LuaField::operator [](const char* key) const {
	return LuaField(this, key);
}

LuaField LuaField::operator [](int index) const {
	return LuaField(this, index);
}

void LuaField::operator =(const LuaField& field) {
	LUAWRAP_ASSERT(field.luastate() == L);

	field.push();
	pop();
}

void LuaField::operator =(const LuaValue& value) {
	LUAWRAP_ASSERT(value.luastate() == L);

	value.push();
	pop();
}

LuaField::operator LuaValue() const {
	push();
	return LuaValue::pop_value(L);
}

/*****************************************************************************
 *                         Lua API convenience methods                       *
 *****************************************************************************/

void LuaField::bind_function(lua_CFunction luafunc) const {
	lua_pushcfunction(L, luafunc);
	pop();
}

void LuaField::error_and_pop(const std::string& expected_type) const {
	const char* repr = lua_tostring(L, -1);
	std::string obj_repr = repr ? repr : luaL_typename(L, -1);
	lua_pop(L, 1);
	luawrap::conversion_error(expected_type, index_path(), obj_repr);
}

bool LuaField::isnil() const {
	push();
	bool nil = lua_isnil(L, -1);
	lua_pop(L, 1);
	return nil;
}

void LuaField::newtable() const {
	lua_newtable(L);
	pop();
}

void LuaField::set_nil() const {
	lua_pushnil(L);
	pop();
}

void* LuaField::to_userdata() const {
	push();
	void* userdata = lua_touserdata(L, -1);
	if (!userdata) {
		error_and_pop("userdata");
	}
	lua_pop(L, 1);
	return userdata;
}

double LuaField::to_num() const {
	push();
	if (!lua_isnumber(L, -1)) {
		error_and_pop("double");
	}
	double num = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return num;
}

int LuaField::to_int() const {
	push();

	//TODO: Evaluate whether full integer checking is necessary
	double num = lua_tonumber(L, -1);
	int integer = (int) num;
	if (!lua_isnumber(L, -1) || num != integer) {
		error_and_pop("integer");
	}
	lua_pop(L, 1);
	return integer;
}

bool LuaField::to_bool() const {
	push();
	if (!lua_isboolean(L, -1)) {
		error_and_pop("boolean");
	}
	bool boolean = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return boolean;
}

const char* LuaField::to_str() const {
	push();
	if (!lua_isstring(L, -1)) {
		error_and_pop("string");
	}
	const char* str = lua_tostring(L, -1);
	lua_pop(L, 1);
	return str;
}

int LuaField::objlen() const {
	push();
	if (!lua_istable(L, -1)) {
		error_and_pop("table");
	}
	int len = lua_objlen(L, -1);
	lua_pop(L, 1);
	return len;
}

const LuaField& luawrap::ensure_table(const LuaField& field) {
	lua_State* L = field.luastate();
	field.raw_push();
	if (!lua_istable(L, -1)) {
		lua_newtable(L);
		field.raw_pop();
	}
	lua_pop(L, 1); // Field
	return field;
}
