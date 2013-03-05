#include <string>
#include <cstring>
#include <cstdlib>

#include <lua.hpp>

#include <luawrap/LuaValue.h>
#include <luawrap/luawraperror.h>

#include "luawrapassert.h"

using namespace _luawrap_private;

namespace luawrap {
	namespace _private {
		//Use RAII to pop the lua stack after return:
		struct PopHack {
			lua_State* L;
			PopHack(lua_State* L) :
					L(L) {
			}
			~PopHack() {
				lua_pop(L, 1);
			}
		};
	}
}

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

		void pop() {
			int valueidx = lua_gettop(L);
			lua_pushlightuserdata(L, this); /* push address as key */
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

void LuaValue::pop() {
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

bool LuaValue::isnil() const {
	impl->push();
	bool nilval = lua_isnil(impl->L, -1);
	lua_pop(impl->L, 1);
	return nilval;
}

void luafield_pop(lua_State* L, const LuaValue& value, const char* key) {
	value.push();
	LUAWRAP_ASSERT(!lua_isnil(L, -1));

	lua_pushvalue(L, -2);
	lua_setfield(L, -2, key);
	/*Pop table and value*/
	lua_pop(L, 2);
}
void luafield_push(lua_State* L, const LuaValue& value, const char* key) {
	value.push(); /*Get the associatedb lua table*/
	LUAWRAP_ASSERT(!lua_isnil(L, -1));

	int tableind = lua_gettop(L);
	lua_getfield(L, tableind, key);
	lua_replace(L, tableind);
}

namespace _luawrap_private {

	void _LuaField::push() const {
		luafield_push(value.luastate(), value, key);
	}

	void _LuaField::pop() const {
		luafield_pop(value.luastate(), value, key);
	}

	void _LuaField::operator =(const LuaValue & value) {
		value.push();
		pop();
	}

	void _LuaField::operator =(lua_CFunction func) {
		lua_pushcfunction(value.luastate(), func);
		pop();
	}

	void _LuaField::operator =(const char* str) {
		lua_pushstring(value.luastate(), str);
		pop();
	}

	LuaValue _LuaField::ensure_table() const {
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

	void _LuaField::operator =(const LuaStackValue& value) {
		value.push();
		pop();
	}

	bool _LuaField::isnil() const {
		push();
		bool nil = lua_isnil(value.luastate(), -1);
		lua_pop(value.luastate(), 1);
		return nil;
	}

	void _LuaField::bind_function(lua_CFunction func) {
		lua_pushcfunction(value.luastate(), func);
		pop();
	}

}

_luawrap_private::_LuaField::operator LuaValue() {
	push();
	return LuaValue::pop_value(value.luastate());
}

void _LuaField::operator =(const _LuaField & field) {
	field.push();
	pop();
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

			luawrap::error(failmsg.c_str());
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

int LuaValue::objlen() const {
	push();
	int len = lua_objlen(luastate(), -1);
	lua_pop(luastate(), 1);
	return len;
}

LuaValue LuaValue::operator [](int idx) const {
	push();
	lua_rawgeti(luastate(), -1, idx);
	LuaValue ret = LuaValue::pop_value(luastate());
	lua_pop(luastate(), 1);
	return ret;
}

LuaValue::LuaValue(const _luawrap_private::_LuaField& field) {
	impl = new _LuaValueImpl(field.value.luastate());
	field.push();
	pop();
}

LuaValue LuaValue::pop_value(lua_State* L) {
	luawrap::_private::PopHack delayedpop(L);
	return LuaValue(L, -1);
}

