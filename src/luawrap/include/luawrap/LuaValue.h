/**
 * LuaValue.h:
 *  Represents a value in the lua registry.
 *  This is a table in lua_State* that cannot be altered by lua code, and thus
 *  can safely be used for internal purposes.
 *
 *  Provides a proxy class for useful table operations, eg luavalue["table_key"] = somevalue;
 *  A number of conversions are supported, however if you want 'somevalue' to be some arbitrary type
 *  you must also include luawrap.h.
 */

#ifndef LUAWRAP_LUAVALUE_H_
#define LUAWRAP_LUAVALUE_H_

#include <string>

#include <lua.hpp>
#include <luawrap/config.h>
#include <luawrap/LuaStackValue.h>
#include <luawrap/LuaField.h>

class LuaValue;

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

namespace _luawrap_private {
	struct _LuaValueImpl;
}

class LuaValue {
public:
	LuaValue(lua_State* L, const char* global);
	LuaValue(lua_State* L, int pos);
	LuaValue(const LuaStackValue& svalue);
	LuaValue(lua_State* L);

	LuaValue(const LuaValue& value);

	LuaValue();
	~LuaValue();

	void init(lua_State* L);

	void newtable() const;

	void operator=(const LuaValue& value);

	void pop();
	void set(int pos);
	void clear();

	void push() const;
	bool empty() const;
	bool isnil() const;

	// Convert to any type
	template<typename T>
	T as() const;
	template<typename T>
	T defaulted(const char* key, const T& value) const;

	// Mainly for low-level routines, do not depend on too heavily
	lua_State* luastate() const;

	/* NB: it is unsafe to have 'std::string& key' be const here!
	 * This would result potentially in a char* ptr being used outside of its scope */
	LuaField operator[](std::string& key) const;
	LuaField operator[](const char* key) const;
	LuaField operator[](int index) const;

	int objlen() const;

	bool operator==(const LuaValue& o) const;
	bool operator!=(const LuaValue& o) const;

	static LuaValue pop_value(lua_State* L);

private:
	_luawrap_private::_LuaValueImpl* impl;
};

namespace luawrap {
	LuaValue eval(lua_State* L, const std::string& code);
}

#endif /* LUAWRAP_LUAVALUE_H_ */
