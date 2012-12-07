/*
 * luawrap.h:
 *  Flexible wrapping library.
 *  Revolves around three operations for each type:
 *   - push: push on lua stack
 *   - get: get from lua index
 *   - check: check that index is desired object
 */

#ifndef LUAWRAP_H_
#define LUAWRAP_H_

extern "C" {
#include <lua/lua.h>
}

#include "../lua/LuaValue.h"

namespace luawrap {
namespace _private {
template<class C>
struct TypeImpl;
}
}

namespace luawrap {

template<typename T>
inline void push(lua_State* L, const T& val) {
	_private::TypeImpl<T>::push(L, val);
}

template<typename T>
inline T get(lua_State* L, int idx) {
	return _private::TypeImpl<T>::get(L, idx);
}

template<typename T>
inline bool check(lua_State* L, int idx) {
	return _private::TypeImpl<T>::check(L, idx);
}

// Utility operators:
template<typename T>
inline void get(lua_State* L, int idx, T& val) {
	val = get<T>(L, idx);
}

template<typename T>
inline T pop(lua_State* L) {
	T val = _private::TypeImpl<T>::get(L, -1);
	lua_pop(L, 1);
	return val;
}

template<typename T>
inline LuaValue value(lua_State* L, const T& val) {
	push<T>(L, val);
	LuaValue lval;
	lval.pop(L);
	return lval;
}

}

#include "predefined_helper.h"

#endif /* LUAWRAP_H_ */
