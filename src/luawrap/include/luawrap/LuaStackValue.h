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

struct lua_State;
class LuaValue;
class LuaStackValue;

typedef int (*lua_CFunction)(lua_State *L);

namespace _luawrap_private {
	class _LuaField;

	struct _LuaStackField {
		friend class ::LuaStackValue;

		_LuaStackField(const LuaStackValue& value, const char* key) :
				value(value), key(key) {
		}
		void push() const;
		void pop() const;

		template<typename T>
		T as();

		operator LuaValue();
		operator int();
		operator double();
		operator const char*();
		operator std::string();

		// get if not nil
		template<typename T>
		void optionalget(T& value);

		template<typename T>
		T defaulted(const T& value);

		template<typename T>
		void operator=(const T& value);

		void operator=(const LuaValue& value);

		void operator=(const char* str);
		void operator=(lua_CFunction func);
		void operator=(const _LuaField& field);
		void operator=(const _LuaStackField& field);

		bool isnil() const;

		LuaValue ensure_table() const;

		template<typename Function>
		void bind_function(Function func);
		void bind_function(lua_CFunction func);

		template<typename T, typename V> void bind_getter(V T::*member);
		template<typename T, typename V> void bind_setter(V T::*member);

	private:
		const LuaStackValue& value;
		const char* key;

		_LuaStackField(const _LuaStackField& field) :
				value(field.value), key(field.key) {
		}
	};

}

class LuaStackValue {
public:
	LuaStackValue(lua_State* L, int idx);
	LuaStackValue();

	void push() const;

	bool empty() const;

	lua_State* luastate() const {
		return L;
	}

	int index() const {
		return idx;
	}

	// Convert to any type
	template<typename T>
	T as() const;

	_luawrap_private::_LuaStackField operator[](const char* key) const {
		return _luawrap_private::_LuaStackField(*this, key);
	}

	//NB: it is unsafe to have 'std::string& key' be const here!
	//This would result potentially in a char* ptr being used outside of its scope
	_luawrap_private::_LuaStackField operator[](std::string& key) const {
		return operator[](key.c_str());
	}

	int objlen() const;
	LuaValue operator[](int idx) const;

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
