#include <string>
#include <cstring>
#include <cstdlib>

#include <lua.hpp>

#include <luawrap/LuaValue.h>

#include "luawrapassert.h"

using namespace _luawrap_private;

namespace _luawrap_private {

	struct _LuaValueImpl {
		_LuaValueImpl() :
				L(NULL), refcount(1) {
		}
		~_LuaValueImpl() {
			if (L) {
				deinitialize(L);
			}
		}

		void set(lua_State* L, int pos) {
			lua_pushvalue(L, pos); /*push value*/
			this->L = L;
			int valueidx = lua_gettop(L);
			lua_pushlightuserdata(L, this); /* push address as key */
			lua_insert(L, -2);
			lua_settable(L, LUA_REGISTRYINDEX);
		}

		void deinitialize(lua_State* L) {
			LUAWRAP_ASSERT(L == this->L);
			lua_pushlightuserdata(L, this); /* push address as key */
			lua_pushnil(L); /* push nil as value */
			lua_settable(L, LUA_REGISTRYINDEX);
			this->L = NULL;
		}

		void push(lua_State* L) const {
			LUAWRAP_ASSERT(L == this->L);
			lua_pushlightuserdata(L, (void*)this); /* push address as key */
			lua_gettable(L, LUA_REGISTRYINDEX);
		}

		void pop(lua_State* L) {
			this->L = L;
			int valueidx = lua_gettop(L);
			lua_pushlightuserdata(L, this); /* push address as key */
			lua_pushvalue(L, valueidx); /*Clone value*/
			lua_settable(L, LUA_REGISTRYINDEX);

			lua_pop(L, 1);
			/*Pop value*/
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
	impl = NULL;
	lua_getglobal(L, global);
	set(L, -1);
	lua_pop(L, 1);
}

LuaValue::LuaValue(lua_State* L, int pos) {
	impl = NULL;
	set(L, pos);
}

LuaValue::LuaValue() {
	impl = NULL;
}

LuaValue::~LuaValue() {
	deref(impl);
}

void LuaValue::table_initialize(lua_State* L) {
	if (!impl)
		impl = new _LuaValueImpl();
	lua_newtable(L);
	impl->pop(L);
}

void LuaValue::deinitialize(lua_State* L) {
	if (impl) {
		impl->deinitialize(L);
	}
}

void LuaValue::push(lua_State* L) const {
	if (!impl)
		lua_pushnil(L);
	else
		impl->push(L);
}

void LuaValue::set(lua_State *L, int pos) {
	if (!impl)
		impl = new _LuaValueImpl();
	impl->set(L, pos);
}

void LuaValue::pop(lua_State* L) {
	if (!impl)
		impl = new _LuaValueImpl();
	impl->pop(L);
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
	return impl == NULL;
}

bool LuaValue::isnil(lua_State* L) {
	if (!impl) {
		return true;
	}
	impl->push(L);
	bool nilval = lua_isnil(L, -1);
	lua_pop(L, 1);
	return nilval;
}

void luafield_pop(lua_State* L, const LuaValue& value, const char* key) {
	value.push(L);
	LUAWRAP_ASSERT(!lua_isnil(L, -1));

	lua_pushvalue(L, -2);
	lua_setfield(L, -2, key);
	/*Pop table and value*/
	lua_pop(L, 2);
}
void luafield_push(lua_State* L, const LuaValue& value, const char* key) {
	value.push(L); /*Get the associated lua table*/
	LUAWRAP_ASSERT(!lua_isnil(L, -1));

	int tableind = lua_gettop(L);
	lua_getfield(L, tableind, key);
	lua_replace(L, tableind);
}

LuaValue::LuaValue(const _LuaFieldValue & cstrfield) {
	impl = new _LuaValueImpl();
	cstrfield.push();
	impl->pop(cstrfield.L);
}

namespace _luawrap_private {

	void _LuaFieldValue::operator =(const LuaValue & value) {
		value.push(L);
		pop();
	}

	void _LuaFieldValue::operator =(lua_CFunction func) {
		lua_pushcfunction(L, func);
		pop();
	}

	void _LuaFieldValue::operator =(const char* value) {
		lua_pushstring(L, value);
		pop();
	}

}

_luawrap_private::_LuaFieldValue::operator LuaValue() {
	push();
	LuaValue val(L, -1);
	lua_pop(L, 1);
	return val;
}

void _LuaFieldValue::operator =(const _LuaFieldValue & field) {
	LUAWRAP_ASSERT(L == field.L);
	field.push();
	pop();
}

bool LuaValue::operator ==(const LuaValue & o) const {
	return impl == o.impl;
}

bool LuaValue::operator !=(const LuaValue & o) const {
	return impl != o.impl;
}

LuaValue LuaValue::globals(lua_State* L) {
	return LuaValue(L, LUA_GLOBALSINDEX);
}

