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

/*****************************************************************************
 *                          Constructors                                     *
 *****************************************************************************/
enum {
	STRING_INDEX = 0, INTEGER_INDEX = 1
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

LuaField::LuaField(LuaField* parent, const char* index) {
	Index ind;
	Parent par;
	par.field = parent;
	ind.string = index;
	init(parent->L, STRING_INDEX, FIELD_PARENT, ind, par);
}

LuaField::LuaField(LuaField* parent, int index) {
	Index ind;
	Parent par;
	par.field = parent;
	ind.integer = index;
	init(parent->L, INTEGER_INDEX, FIELD_PARENT, ind, par);
}

static void adjust_stack_index(lua_State* L, int& stackidx) {
	if (stackidx > LUA_REGISTRYINDEX + 100 && stackidx < 1) {
		stackidx+= lua_gettop(L) + 1; // Convert relative negative position to 'absolute' stack position
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
	if (!_parent_type) {
		_parent.field->index_path(str);
		if (_index_type == STRING_INDEX) {
			str += "[\"";
			str += _index.string;
			str += "\"]";
		} else {
			char buff[32];
			sprintf(buff, "[%d]", _index.integer);
			str += buff;
		}
	}
}

/*****************************************************************************
 *                         Utility methods                                   *
 *****************************************************************************/

void LuaField::push() const {
	push_parent();
	if (_index_type == STRING_INDEX) {
		/* String index */
		lua_getfield(L, -1, _index.string);
	} else {
		lua_pushinteger(L, _index.integer);
		lua_gettable(L, -2);
	}

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
		_parent.field->push();
	}

	if (lua_isnil(L, -1)) {
		handle_nil_parent();
	}
}

void LuaField::pop() const {
	push_parent();

	if (_index_type == STRING_INDEX) {
		lua_pushstring(L, _index.string);
	} else {
		lua_pushinteger(L, _index.integer);
	}

	/* Value to pop */
	lua_pushvalue(L, -3);
	/* Parent */
	lua_gettable(L, -3);
	/* Pop parent and value to pop*/
	lua_pop(L, 2);
}

