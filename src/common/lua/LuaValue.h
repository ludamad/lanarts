/**
 * LuaValue.h:
 *  Represents a value in the lua registry.
 *  This is a table in lua_State* that cannot be altered by lua code, and thus
 *  can safely be used for internal purposes.
 */

#ifndef LCOMMON_LUAVALUE_H_
#define LCOMMON_LUAVALUE_H_

#include <string>

struct lua_State;
class SerializeBuffer;
class LuaValue;

typedef int (*lua_CFunction)(lua_State *L);

void luafield_pop(lua_State* L, const LuaValue& value, const char* key);

inline void luafield_pop(lua_State* L, const LuaValue& value,
		const std::string& key) {
	luafield_pop(L, value, key.c_str());
}

void luafield_push(lua_State* L, const LuaValue& value, const char* key);

inline void luafield_push(lua_State* L, const LuaValue& value,
		const std::string& key) {
	luafield_push(L, value, key.c_str());
}

namespace LCommonPrivate {

struct _LuaValueImpl;

struct _LuaFieldValue {
	_LuaFieldValue(lua_State* L, const LuaValue& value, const char* key) :
			L(L), value(value), key(key) {
	}
	void push() const {
		luafield_push(L, value, key);
	}
	void pop() const {
		luafield_pop(L, value, key);
	}

	lua_State* L;
	const LuaValue& value;
	const char* key;
	template<typename T>
	operator T();

	template<typename T>
	void operator=(const T& value);
	void operator=(const LuaValue& value);
	void operator=(lua_CFunction func);
	void operator=(const _LuaFieldValue& field);

private:
	friend class ::LuaValue;
	_LuaFieldValue(const _LuaFieldValue& field) :
			L(field.L), value(field.value), key(field.key) {
	}
};

}

class LuaValue {
public:
	LuaValue(lua_State* L, const char* global);
	LuaValue(const LuaValue& value);
	LuaValue(lua_State* L, int pos);
	LuaValue(const LCommonPrivate::_LuaFieldValue& cstrfield);
	LuaValue();
	~LuaValue();

	void operator=(const LuaValue& value);

	void deinitialize(lua_State* L);
	void push(lua_State* L) const;
	void pop(lua_State* L);
	void set(lua_State* L, int pos);
	bool empty() const;

	void table_initialize(lua_State* L);

	void serialize(lua_State* L, SerializeBuffer& serializer);
	void deserialize(lua_State* L, SerializeBuffer& serializer);

	LCommonPrivate::_LuaFieldValue get(lua_State* L, const char* key) const {
		return LCommonPrivate::_LuaFieldValue(L, *this, key);
	}
	//NB: it is unsafe to have 'std::string& key' be const here!
	//This would result potentially in a char* ptr being used outside of its scope
	LCommonPrivate::_LuaFieldValue get(lua_State* L, std::string& key) const {
		return get(L, key.c_str());
	}

	bool operator==(const LuaValue& o) const;
	bool operator!=(const LuaValue& o) const;

	bool isnil(lua_State* L);
private:
	LCommonPrivate::_LuaValueImpl* impl;
};

//LuaValue luavalue_eval(lua_State* L, const std::string& code);

#endif /* LCOMMON_LUAVALUE_H_ */
