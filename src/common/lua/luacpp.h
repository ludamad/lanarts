/*
 * luacpp.h:
 *  Wrap the SLB-based implementation of our pushes and gets.
 *	Include eg 'lua_timer' to have the overrides available
 */

#ifndef LUACPP_H_
#define LUACPP_H_

#include <SLB/Type.hpp>
#include <SLB/PushGet.hpp>

#include "../lcommon_defines.h"

#include "LuaValue.h"

template<typename T>
inline void luacpp_push(lua_State* L, const T& value) {
	SLB::push<T>(L, value);
}
inline void luacpp_push(lua_State* L, const char* key) {
	lua_pushstring(L, key);
}

template<typename T>
inline LuaValue luacpp(lua_State* L, const T& value) {
	LuaValue ret;
	luacpp_push(L, value);
	ret.pop(L);

	return ret;
}

template<typename T>
inline T luacpp_get(lua_State* L, int pos) {
	return SLB::get<T>(L, pos);
}

template<typename T>
inline void luacpp_get(lua_State* L, int pos, T& value) {
	LCOMMON_ASSERT(SLB::check<T>(L, pos));
	value = SLB::get<T>(L, pos);
}

template<typename T>
inline void luafield_set(lua_State* L, const LuaValue& value, const char* key,
		const T& t) {
	luacpp_push(L, t);
	luafield_pop(L, value, key);
}
template<typename T>
inline void luafield_get(lua_State* L, const LuaValue& value, const char* key,
		T& t) {
	luafield_push(L, value, key);
	luacpp_get(L, -1, t);
	lua_pop(L, 1);
}

namespace LCommonPrivate {

template<typename T>
inline _LuaFieldValue::operator T() {
	T t;
	luafield_get(L, value, key, t);
	return t;
}

template<typename T>
inline void _LuaFieldValue::operator=(const T& t) {
	luafield_set(L, value, key, t);
}

}

#endif /* LUACPP_H_ */
