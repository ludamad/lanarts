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

	void operator=(const LuaValue& value);

	void pop() const;
	void clear();

	void push() const;
	bool empty() const;

	// Convert to any type
	template<typename T>
	T as() const;

	// We can convert to a forwarding LuaField for API convenience
	operator LuaField() const ;

	// Mainly for low-level routines, do not depend on too heavily
	lua_State* luastate() const;

	/* NB: it is unsafe to have 'std::string& key' be const here!
	 * This would result potentially in a char* ptr being used outside of its scope */
	LuaField operator[](std::string& key) const;
	LuaField operator[](const char* key) const;
	LuaField operator[](int index) const;

	/* Lua api convenience methods*/
	void set(int pos);
	void set_nil() const;
	bool has(const char* key) const;
	void newtable() const;
	bool isnil() const;
	void* to_userdata() const;
	double to_num() const;
	bool to_bool() const;
	int to_int() const;
	const char* to_str() const;
	int objlen() const;
	LuaValue metatable() const;
	void set_metatable(const LuaField& metatable) const;

	bool operator==(const LuaValue& o) const;
	bool operator!=(const LuaValue& o) const;

	static LuaValue pop_value(lua_State* L);
	static LuaValue newtable(lua_State* L);

private:
	_luawrap_private::_LuaValueImpl* impl;
};

namespace luawrap {

	const LuaField& ensure_table(const LuaField& field);

	template<typename LuaWrapper, typename T>
	T set_if_nil(const LuaWrapper& wrapper, const T& value) {
		if (wrapper.isnil()) {
			wrapper = value;
			return value;
		}

		return wrapper.template as<T>();
	}
	template<typename LuaWrapper, typename T>
	T set_if_nil(const LuaWrapper& wrapper, const char* key, const T& value) {
		LuaField field = wrapper[key];
		if (field.isnil()) {
			field = value;
			return value;
		}
		return field.as<T>();
	}

	template<typename LuaWrapper, typename T>
	inline T defaulted(const LuaWrapper& wrapper, const T& value) {
		if (wrapper.isnil()) {
			return value;
		}
		return wrapper.template as<T>();
	}

	template<typename LuaWrapper, typename T>
	inline T defaulted(const LuaWrapper& wrapper, const char* key, const T& value) {
		LuaField field = wrapper[key];
		if (field.isnil()) {
			return value;
		}
		return field.as<T>();
	}

	LuaValue eval(lua_State* L, const std::string& code);
}

#endif /* LUAWRAP_LUAVALUE_H_ */
