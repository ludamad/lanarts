/*
 * luacpp_wrap_members.h:
 *  Wrap CPP member get/set in lua functions.
 */

#ifndef LUACPP_WRAP_MEMBERS_H_
#define LUACPP_WRAP_MEMBERS_H_

#include "luacpp.h"

template<typename T, typename V, V T::* member>
static int luacpp_setmember(lua_State* L) {
	T& object = luacpp_get<T&>(L, 1);
	// 2nd arg is 'key'
	luacpp_get<V>(L, 3, object.*member);
	return 0;
}

template<typename T, typename V, V T::* member>
static int luacpp_getmember(lua_State* L) {
	T& object = luacpp_get<T&>(L, 1);
	luacpp_push<V>(L, object.*member);
	return 1;
}

template<typename T, typename V, V& (T::*member)()>
static int luacpp_setmemberfn(lua_State* L) {
	T& object = luacpp_get<T&>(L, 1);
	// 2nd arg is 'key'
	luacpp_get<V>(L, 3, (object.*member)());
	return 0;
}

template<typename T, typename V, void (T::*member)(V)>
static int luacpp_setter(lua_State* L) {
	T& object = luacpp_get<T&>(L, 1);
	// 2nd arg is 'key'
	(object.*member)(luacpp_get<V>(L, 3));
	return 0;
}

template<typename T, typename V, V& (T::*member)()>
static int luacpp_getmemberfn(lua_State* L) {
	T& object = luacpp_get<T&>(L, 1);
	luacpp_push<V>(L, (object.*member)());
	return 1;
}

#endif /* LUACPP_WRAP_MEMBERS_H_ */
