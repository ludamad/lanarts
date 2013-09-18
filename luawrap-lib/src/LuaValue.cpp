#include <string>
#include <cstring>
#include <cstdlib>

#include <lua.hpp>

#include <luawrap/LuaField.h>
#include <luawrap/LuaValue.h>
#include <luawrap/luawraperror.h>
#include <luawrap/luawrap.h>

#include "luawrapassert.h"

using namespace _luawrap_private;

namespace _luawrap_private {

	struct _LuaValueImpl {
		_LuaValueImpl(lua_State* L) :
				L(L), refcount(1) {
		}
		~_LuaValueImpl() {
			if (L) {
				clear();
			}
		}

		void set(int pos) {
			lua_pushvalue(L, pos); /*push value*/
			int valueidx = lua_gettop(L);
			lua_pushlightuserdata(L, this); /* push address as key */
			lua_insert(L, -2);
			lua_settable(L, LUA_REGISTRYINDEX);
		}

		void clear() {
			lua_pushlightuserdata(L, this); /* push address as key */
			lua_pushnil(L); /* push nil as value */
			lua_settable(L, LUA_REGISTRYINDEX);
			this->L = NULL;
		}

		void push() const {
			lua_pushlightuserdata(L, (void*)this); /* push address as key */
			lua_gettable(L, LUA_REGISTRYINDEX);
		}

		void pop() const {
			int valueidx = lua_gettop(L);
			lua_pushlightuserdata(L, (void*)this); /* push address as key */
			lua_pushvalue(L, valueidx); /*Clone value*/
			lua_settable(L, LUA_REGISTRYINDEX);

			lua_pop(L, 1);
			/*Pop value*/
		}

		bool is_uniquely_referenced() {
			return refcount == 1;
		}

		lua_State* L;
		size_t refcount;
	};

}

static void deref(_LuaValueImpl* impl) {
	if (impl && --impl->refcount == 0)
		delete impl;
}

LuaValue::LuaValue(lua_State* L, const char* global) {
	impl = new _LuaValueImpl(L);
	lua_getglobal(L, global);
	set(-1);
	lua_pop(L, 1);
}

LuaValue::LuaValue(lua_State* L, int pos) {
	impl = new _LuaValueImpl(L);
	set(pos);
}

LuaValue::LuaValue(const LuaStackValue& svalue) {
	impl = new _LuaValueImpl(svalue.luastate());
	set(svalue.index());
}

LuaValue::LuaValue() {
	impl = NULL;
}

LuaValue::~LuaValue() {
	deref(impl);
}

void LuaValue::newtable() const {
	lua_newtable(impl->L);
	impl->pop();
}

void LuaValue::clear() {
	if (impl) {
		if (impl->is_uniquely_referenced()) {
			impl->clear();
		} else {
			deref(impl);
			impl = NULL;
		}
	}
}

void LuaValue::push() const {
	impl->push();
}

void LuaValue::set(int pos) {
	impl->set(pos);
}

void LuaValue::pop() const {
	impl->pop();
}

LuaValue::LuaValue(const LuaValue & value) {
	impl = value.impl;
	if (impl) {
		impl->refcount++;
	}
}

void LuaValue::operator =(const LuaValue & value) {
	deref(impl);
	impl = value.impl;
	if (impl) {
		impl->refcount++;
	}
}

bool LuaValue::empty() const {
	return impl == NULL || impl->L == NULL;
}

bool LuaValue::has(const char* key) const {
	return !(*this)[key].isnil();
}

bool LuaValue::isnil() const {
	impl->push();
	bool nilval = lua_isnil(impl->L, -1);
	lua_pop(impl->L, 1);
	return nilval;
}

bool LuaValue::operator ==(const LuaValue & o) const {
	return impl == o.impl;
}

bool LuaValue::operator !=(const LuaValue & o) const {
	return impl != o.impl;
}

LuaValue::LuaValue(lua_State* L) {
	impl = new _LuaValueImpl(L);
}

void LuaValue::init(lua_State* L) {
	if (impl) {
		if (impl->is_uniquely_referenced()) {
			lua_pushnil(L);
			impl->pop();
			return;
		}
		deref(impl);
	}
	impl = new _LuaValueImpl(L);
}

static std::string format_expression_string(const std::string& str) {
	const char prefix[] = "return ";
	if (strncmp(str.c_str(), prefix, sizeof(prefix)) == 0)
		return str;
	return "return " + str;
}

namespace luawrap {

	LuaValue eval(lua_State* L, const std::string& code) {
		if (code.empty()) {
			return LuaValue();
		}
		std::string expr = format_expression_string(code);
		int ntop = lua_gettop(L);

		if (luaL_dostring(L, expr.c_str())) {
			std::string failmsg;

			failmsg += "\nWhen running ... \n";
			failmsg += code;
			failmsg += "\n... an error occurred in lua's runtime:\n";
			failmsg += lua_tostring(L, -1);

			luawrap::error(failmsg);
		}

		LUAWRAP_ASSERT(lua_gettop(L) - ntop == 1);

		LuaValue val(L, -1);
		lua_pop(L, 1);

		return val;
	}

}

lua_State* LuaValue::luastate() const {
	return impl->L;
}

LuaField LuaValue::operator [](std::string& key) const {
	return LuaField(impl->L, (void*)impl, key.c_str());
}

LuaField LuaValue::operator [](const char* key) const {
	return LuaField(impl->L, (void*)impl, key);
}

LuaField LuaValue::operator [](int index) const {
	return LuaField(impl->L, (void*)impl, index);
}

LuaValue LuaValue::metatable() const {
	push();
	if (!lua_getmetatable(impl->L, -1)) {
		lua_pushnil(impl->L);
	}
	LuaValue table(impl->L, -1);
	lua_pop(impl->L, 2);
	return table;
}

void LuaValue::set_nil() const {
	lua_pushnil(luastate());
	pop();
}

static void error_and_pop(lua_State* L, const std::string& expected_type) {
	const char* repr = lua_tostring(L, -1);
	std::string obj_repr = repr ? repr : "nil";
	std::string type = luaL_typename(L, -1);
	lua_pop(L, 1);
	luawrap::conversion_error(type, "", obj_repr);
}

void* LuaValue::to_userdata() const {
	lua_State* L = luastate();
	push();
	void* userdata = lua_touserdata(L, -1);
	if (!userdata) {
		error_and_pop(L, "userdata");
	}
	lua_pop(L, 1);
	return userdata;
}

double LuaValue::to_num() const {
	lua_State* L = luastate();
	push();
	if (!lua_isnumber(L, -1)) {
		error_and_pop(L, "double");
	}
	double num = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return num;
}


int LuaValue::to_int() const {
	lua_State* L = luastate();
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

bool LuaValue::to_bool() const {
	lua_State* L = luastate();
	push();
	if (!lua_isboolean(L, -1)) {
		error_and_pop(L, "boolean");
	}
	bool boolean = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return boolean;
}

const char* LuaValue::to_str() const {
	lua_State* L = luastate();
	push();
	if (!lua_isstring(L, -1)) {
		error_and_pop(L, "string");
	}
	const char* str = lua_tostring(L, -1);
	lua_pop(L, 1);
	return str;
}

int LuaValue::objlen() const {
	lua_State* L = luastate();
	push();
	if (!lua_istable(L, -1)) {
		error_and_pop(L, "table");
	}
	int len = lua_objlen(L, -1);
	lua_pop(L, 1);
	return len;
}

LuaValue::operator LuaField() const {
	return LuaField(impl->L, (void*)impl);
}

void LuaValue::set_metatable(const LuaField& metatable) const {
	push();
	metatable.push();
	lua_setmetatable(impl->L, -2);
	lua_pop(impl->L, 1);
}

LuaValue LuaValue::pop_value(lua_State* L) {
	luawrap::_private::PopHack delayedpop(L);
	return LuaValue(L, -1);
}

LuaValue LuaValue::newtable(lua_State* L) {
	LuaValue val(L);
	val.newtable();
	return val;
}
