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
		operator T();

		template<typename T>
		void operator=(const T& value);

		void operator=(const LuaValue& value);

		void operator=(const char* str);
		void operator=(lua_CFunction func);
		void operator=(const _LuaField& field);
		void operator=(const _LuaStackField& field);

		template<typename Function>
		void bind_function(Function func);

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

	void push(lua_State* L) const;

	bool empty() const;

	lua_State* luastate() const {
		return L;
	}

	int index() const {
		return idx;
	}

	_luawrap_private::_LuaStackField operator[](const char* key) const {
		return _luawrap_private::_LuaStackField(*this, key);
	}

	//NB: it is unsafe to have 'std::string& key' be const here!
	//This would result potentially in a char* ptr being used outside of its scope
//	_luawrap_private::_LuaStackField operator[](std::string& key) const {
//		return operator[](key.c_str);
//	}

private:
	lua_State* L;
	int idx;
};

typedef LuaStackValue LuaSpecialValue;

namespace luawrap {
	LuaSpecialValue globals(lua_State* L);
}

#endif /* LUAWRAP_LUASTACKVALUE_H_ */
