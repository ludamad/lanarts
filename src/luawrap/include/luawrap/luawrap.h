/*
 * luawrap.h:
 *  Flexible wrapping library.
 *  Revolves around three operations for each type:
 *   - push: push on lua stack
 *   - get: get from lua index
 *   - check: check that index is desired object
 *
 *  Basic idea -> const T& and T are value types and operate on callbacks bound to T
 *  T* and T& are reference types and operate on callbacks bound to T*
 *
 */

#ifndef LUAWRAP_LUAWRAP_H_
#define LUAWRAP_LUAWRAP_H_

#include <cstdio>

#include <lua.hpp>

#include <luawrap/config.h>

#include <luawrap/LuaValue.h>
#include <luawrap/LuaField.h>
#include <luawrap/LuaStackValue.h>
#include <luawrap/luawraperror.h>

// Include internal header
#include "../../src/pushget_helper.h"

namespace luawrap {
	namespace _private {
		template<class C>
		struct TypeImpl;

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

namespace luawrap {
	void dostring(lua_State* L, const char* code);
	void dofile(lua_State* L, const char* filename);

	template<typename T>
	inline void push(lua_State* L, const T& val) {
		_private::PushGetCheckWrap<T>::push(L, val);
	}

	template<typename T>
	inline typename _private::PushGetCheckWrap<T>::RetType get(lua_State* L,
			int idx) {
		return _private::PushGetCheckWrap<T>::get(L, idx);
	}

	template<typename T>
	inline bool check(lua_State* L, int idx) {
		return _private::PushGetCheckWrap<T>::check(L, idx);
	}

	// Utility operators:
	template<typename T>
	inline void get(lua_State* L, int idx, T& val) {
		val = get<T>(L, idx);
	}

	template<typename T>
	inline T pop(lua_State* L) {
		_private::PopHack delayedpop(L);
		return _private::TypeImpl<T>::get(L, -1);
	}

	template<typename T>
	inline LuaValue value(lua_State* L, const T& val) {
		push<T>(L, val);
		LuaValue lval(L);
		lval.pop();
		return lval;
	}

	template<typename T>
	inline void setfield(lua_State* L, int idx, const char* key, const T& val) {
		push<T>(L, val);
		lua_setfield(L, idx, key);
	}

	template<typename T>
	inline T getfield(lua_State* L, int idx, const char* key) {
		lua_getfield(L, idx, key);
		return pop<T>(L);
	}

	template<typename T>
	inline void getfield(lua_State* L, int idx, const char* key, T& val) {
		val = getfield<T>(L, idx, key);
	}

	// Get field if value is not nil
	template<typename T>
	inline void getoptfield(lua_State* L, int idx, const char* key, T& val) {
		lua_getfield(L, idx, key);
		if (!lua_isnil(L, -1)) {
			if (!check<T>(L, -1)) {
				std::string err = "Field '" + std::string(key) + "' was not the expected type!\n";
				luawrap::error(err.c_str());
			}
			val = pop<T>(L);
		} else {
			lua_pop(L, 1);
		}
	}
}

// LuaField implementations

template<typename T>
inline void LuaField::operator =(const T& value) {
	luawrap::push<T>(L, value);
	pop();
}

template<typename T>
inline T LuaField::as() const {
	push();
	return luawrap::pop<T>(L);
}

template<typename T>
inline bool LuaField::is() const {
	luawrap::_private::PopHack delayedpop(L);
	push();
	return luawrap::check<T>(L, -1);
}

template<typename T>
inline T LuaStackValue::as() const {
	if (!luawrap::check<T>(L, idx)) {
		luawrap::error("Error converting type!");
	}
	return luawrap::get<T>(L, idx);
}

template<typename T>
inline T LuaValue::as() const {
	push();

	if (!luawrap::check<T>(luastate(), -1)) {
		luawrap::error("Error converting type!");
	}

	return luawrap::pop<T>(luastate());
}

#include "../../src/predefined_helper.h"

#include <luawrap/calls.h>
#include <luawrap/functions.h>
#include <luawrap/members.h>

#endif /* LUAWRAP_LUAWRAP_H_ */
