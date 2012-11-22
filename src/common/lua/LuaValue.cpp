#include <string>
#include <cstring>
#include <cstdlib>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "../SerializeBuffer.h"

#include "LuaValue.h"
#include "lua_serialize.h"

using namespace LCommonPrivate;

static std::string format_expression_string(const std::string& str) {
	const char prefix[] = "return ";
	if (str.empty())
		return str;
	if (strncmp(str.c_str(), prefix, sizeof(prefix)) == 0)
		return str;
	return "return " + str;
}

namespace LCommonPrivate {

struct _LuaValueImpl {
	_LuaValueImpl(const std::string& expr = std::string()) :
			L(NULL), lua_expression(format_expression_string(expr)), refcount(
					1), empty(true) {
	}
	~_LuaValueImpl() {
		if (L) {
			deinitialize(L);
		}
	}

	void set(lua_State* L, int pos) {
		lua_pushvalue(L, pos); /*push value*/
		this->L = L;
		empty = false;
		int valueidx = lua_gettop(L);
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_insert(L, -2);
		lua_settable(L, LUA_REGISTRYINDEX);
	}

	void deinitialize(lua_State* L) {
		if (!empty) {
			LCOMMON_ASSERT(L == this->L);
			lua_pushlightuserdata(L, this); /* push address as key */
			lua_pushnil(L); /* push nil as value */
			lua_settable(L, LUA_REGISTRYINDEX);
			this->L = NULL;
		}
	}

	void initialize(lua_State* L) {
		if (lua_expression.empty())
			return;
		this->L = L;
		empty = false;
		lua_pushlightuserdata(L, this); /* push address as key */
		luaL_dostring(L, lua_expression.c_str());
		lua_settable(L, LUA_REGISTRYINDEX);
		lua_expression.clear();
	}

	void push(lua_State* L) const {
		LCOMMON_ASSERT(L == this->L);
		lua_pushlightuserdata(L, (void*)this); /* push address as key */
		lua_gettable(L, LUA_REGISTRYINDEX);
	}

	void pop(lua_State* L) {
		this->L = L;
		empty = false;
		int valueidx = lua_gettop(L);
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_pushvalue(L, valueidx); /*Clone value*/
		lua_settable(L, LUA_REGISTRYINDEX);

		lua_pop(L, 1);
		/*Pop value*/
	}

	void serialize(lua_State* L, SerializeBuffer& serializer) {

		serializer.write(lua_expression);
		serializer.write(empty);
		push(L);
		lua_serialize(serializer, L, -1);
	}
	void deserialize(lua_State* L, SerializeBuffer& serializer) {
		serializer.read(lua_expression);
		serializer.read(empty);
		lua_deserialize(serializer, L);
		pop(L);
	}

	lua_State* L;
	std::string lua_expression;
	size_t refcount;
	bool empty;
};

}

static void deref(_LuaValueImpl* impl) {
	if (impl && --impl->refcount == 0)
		delete impl;
}

LuaValue::LuaValue(const std::string & expr) {
	if (expr.empty())
		impl = NULL;
	else
		impl = new _LuaValueImpl(expr);
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

void LuaValue::initialize(lua_State* L) {
	if (impl)
		impl->initialize(L);
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
	return impl == NULL || impl->empty;
}

void LuaValue::serialize(lua_State* L, SerializeBuffer& serializer) {
	serializer.write_byte(impl != NULL);
	if (impl) {
		impl->serialize(L, serializer);
	}
}

void LuaValue::deserialize(lua_State* L, SerializeBuffer& serializer) {
// Handle empty LuaValue's:
	int is_on;
	serializer.read_byte(is_on);
	if (!is_on) {
		delete impl;
		impl = NULL;
		return;
	}

	if (!impl) {
		impl = new _LuaValueImpl();
	}
	impl->deserialize(L, serializer);
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
	LCOMMON_ASSERT(!lua_isnil(L, -1));

	lua_pushvalue(L, -2);
	lua_setfield(L, -2, key);
	/*Pop table and value*/
	lua_pop(L, 2);
}
void luafield_push(lua_State* L, const LuaValue& value, const char* key) {
	value.push(L); /*Get the associated lua table*/
	LCOMMON_ASSERT(!lua_isnil(L, -1));

	int tableind = lua_gettop(L);
	lua_getfield(L, tableind, key);
	lua_replace(L, tableind);
}

LuaValue::LuaValue(const _LuaFieldValue & cstrfield) {
	impl = new _LuaValueImpl();
	cstrfield.push();
	impl->pop(cstrfield.L);
}

namespace LCommonPrivate {

void _LuaFieldValue::operator =(const LuaValue & value) {
	value.push(L);
	pop();
}

void _LuaFieldValue::operator =(lua_CFunction func) {
	lua_pushcfunction(L, func);
	pop();
}

}

void _LuaFieldValue::operator =(const _LuaFieldValue & field) {
	LCOMMON_ASSERT(L == field.L);
	field.push();
	pop();
}

bool LuaValue::operator ==(const LuaValue & o) const {
	return impl == o.impl;
}

bool LuaValue::operator !=(const LuaValue & o) const {
	return impl != o.impl;
}

