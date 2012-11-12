/*
 * lua_wrap_util.h:
 *  Functions for going to and from numeric arrays
 */

#ifndef LUA_WRAP_UTIL_H_
#define LUA_WRAP_UTIL_H_

#include <SLB/Type.hpp>
#include <SLB/PushGet.hpp>

extern "C" {
#include <lua/lua.h>
}

template<typename T, typename Num>
inline void lua_push_as_numarray(lua_State* L, const T& value) {
	lua_newtable(L);
	const Num* nums = (const Num*)&value;
	int idx = lua_gettop(L);
	for (int i = 0; i < sizeof(T) / sizeof(Num); i++) {
		lua_pushnumber(L, nums[i]);
		lua_rawseti(L, idx, i + 1);
	}
}

template<typename T, typename Num, int default_value>
inline T lua_get_from_numarray(lua_State* L, int idx) {
	T value;
	Num* nums = (Num*)&value;
	for (int i = 0; i < sizeof(T) / sizeof(Num); i++) {
		lua_rawgeti(L, idx, i + 1);
		if (!lua_isnil(L, -1)) {
			nums[i] = lua_tonumber(L, -1);
		} else {
			nums[i] = default_value;
		}
		lua_pop(L, 1);
	}
	return value;
}
template<typename T, typename Num, int allowed_missing>
inline bool lua_check_as_numarray(lua_State *L, int idx) {
	const int n = sizeof(T) / sizeof(Num);
	const int mincheckn = sizeof(T) / sizeof(Num) - allowed_missing;
	int olen = lua_objlen(L, idx);

	if (!lua_istable(L, idx) || olen < mincheckn || olen > n) {
		return false;
	}
	bool valid = true;
	for (int i = 0; i < mincheckn && valid; i++) {
		lua_rawgeti(L, idx, i + 1);
		if (!lua_isnumber(L, -1)) {
			valid = false;
		}
		lua_pop(L, 1);
	}
	return valid;
}
namespace SLB {
template<typename T, typename V>
inline void table_get(lua_State *L, int idx, const char* key, V& value) {
	lua_pushstring(L, key);
	lua_gettable(L, idx);
	value = V(get<T>(L, -1));
	lua_pop(L, 1);
}
template<typename V>
inline void table_get(lua_State *L, int idx, const char* key, V& value) {
	lua_pushstring(L, key);
	lua_gettable(L, idx);
	value = get<V>(L, -1);
	lua_pop(L, 1);
}
template<typename V>
inline void table_set(lua_State *L, int idx, const char* key, const V& value) {
	lua_pushstring(L, key);
	push(L, value);
	lua_settable(L, idx);
}

}

#include <SLB/Type.hpp>

#define SLB_PRIVATE_TYPE_SPECIALIZATION(ParamTypes, SpecType, T, pushImpl, getImpl, checkImpl) \
	namespace SLB {	\
	namespace Private {	\
	template< ParamTypes > 	\
	struct Type<SpecType> { \
		typedef T GetType; \
		static void push(lua_State *L, const T& p) { \
			pushImpl(L, p); \
		} \
		static T get(lua_State *L, int idx) { \
			return getImpl(L, idx); \
		} \
		static bool check(lua_State *L, int idx) { \
			return checkImpl(L, idx); \
		} \
	}; \
	} \
	}

#define SLB_WRAP_VALUE_TYPE(T, push, get, check) \
		SLB_PRIVATE_TYPE_SPECIALIZATION(,T, T, push, get, check) \
		SLB_PRIVATE_TYPE_SPECIALIZATION(/*Nothing*/, const T &, T, push, get, check)

#define SLB_WRAP_VALUE_TYPE_AS_NUMARR(T, Num) \
		SLB_WRAP_VALUE_TYPE(T, (lua_push_as_numarray<T, Num>), (lua_get_from_numarray<T, Num, 0>), (lua_check_as_numarray<T, Num, 0>))

template<typename T>
inline void slb_tableset(lua_State* L, int idx, const char* key, const T& val) {
	lua_pushstring(L, key);
	SLB::push<T>(L, val);
	lua_rawset(L, (idx < 0 ? idx - 2 : idx));
}

#endif /* LUA_WRAP_UTIL_H_ */
