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

#include <luawrap/config.h>
#include <luawrap/LuaStackValue.h>

struct lua_State;
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

namespace _luawrap_private {

	struct _LuaValueImpl;

	struct _LuaField {
		_LuaField(const LuaValue& value, const char* key) :
				value(value), key(key) {
		}
		void push() const;

		void pop() const;

		const LuaValue& value;
		const char* key;
		template<typename T> operator T();
		operator LuaValue();
		template<typename T> void operator =(const T& value);
		void operator =(const LuaValue& value);
		void operator =(const char* str);
		void operator =(lua_CFunction func);
		void operator =(const _LuaField& field);

		// Registers a CPP function
		template<typename Function> void bind_function(Function func);
	private:
		friend class ::LuaValue;
		_LuaField(const _LuaField& field) :
			value(field.value), key(field.key) {
		}
	};

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

	void newtable();

	void operator=(const LuaValue& value);

	void clear();
	void pop();
	void set(int pos);

	void push() const;
	bool empty() const;
	bool isnil() const;

	// Mainly for low-level routines, do not depend on too heavily
	lua_State* luastate() const;

	_luawrap_private::_LuaField operator[](const char* key) const {
		return _luawrap_private::_LuaField(*this, key);
	}
	//NB: it is unsafe to have 'std::string& key' be const here!
	//This would result potentially in a char* ptr being used outside of its scope
	_luawrap_private::_LuaField operator[](std::string& key) const {
		return operator[](key.c_str());
	}

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
