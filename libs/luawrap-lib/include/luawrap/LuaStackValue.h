/**
 * LuaStackValue.h:
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

#ifndef LUAWRAP_LUASTACKVALUE_H_
#define LUAWRAP_LUASTACKVALUE_H_

#include <string>

#include <lua.hpp>
#include <luawrap/LuaField.h>

class LuaStackValue {
public:
	LuaStackValue(lua_State* L, int idx);
	LuaStackValue();

	void push() const;
	void pop() const;

	bool empty() const;

	lua_State* luastate() const {
		return L;
	}

	int index() const {
		return idx;
	}

	// We can convert to a forwarding LuaField for API convenience
	operator LuaField() const ;

	// Convert to any type
	template<typename T>
	T as() const;

	/* NB: it is unsafe to have 'std::string& key' be const here!
	 * This would result potentially in a char* ptr being used outside of its scope */
	LuaField operator[](std::string& key) const;
	LuaField operator[](const char* key) const;
	LuaField operator[](int index) const;

	/* Lua API convenience methods */
	void set(int pos);
	bool has(const char* key) const;
	void newtable() const;
	void set_nil() const;
	bool isnil() const;
	void* to_userdata() const;
	double to_num() const;
	bool to_bool() const;
	int to_int() const;
	const char* to_str() const;
	int objlen() const;
	LuaValue metatable() const;
	void set_metatable(const LuaField& metatable) const;

private:
	lua_State* L;
	int idx;
};

typedef LuaStackValue LuaSpecialValue;

namespace luawrap {
	LuaSpecialValue globals(lua_State* L);
	LuaSpecialValue registry(lua_State* L);
}

#endif /* LUAWRAP_LUASTACKVALUE_H_ */
