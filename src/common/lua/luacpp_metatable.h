/*
 * luacpp_metatable.h:
 *  Helpers for creating a flexible metatable for lua classes holding C++ data.
 *  Relies on luacpp_get
 */

#ifndef LUACPP_METATABLE_H_
#define LUACPP_METATABLE_H_

#include "luacpp.h"
#include "luacpp_wrap_members.h"

LuaValue luameta_new(lua_State* L, const char* classname);

typedef LuaValue (*luameta_initializer)(lua_State* L);
void luameta_pushcached(lua_State* L, luameta_initializer initfunc);

void luameta_pop_getter(lua_State* L, const LuaValue& value,
		const char* keyname);
void luameta_pop_setter(lua_State* L, const LuaValue& value,
		const char* keyname);

void luameta_pop_method(lua_State* L, const LuaValue& value,
		const char* keyname);

template<typename T, typename V, V T::* member>
static inline void luameta_getter_member(lua_State* L, const LuaValue& value,
		const char* keyname) {
	lua_pushcfunction(L, (&luacpp_getmember<T,V,member>));
	luameta_pop_getter(L, value, keyname);
}

template<typename T, typename V, V T::* member>
static inline void luameta_member(lua_State* L, const LuaValue& value,
		const char* keyname) {
	lua_pushcfunction(L, (&luacpp_setmember<T,V,member>));
	luameta_pop_setter(L, value, keyname);
	luameta_getter_member<T, V, member>(L, value, keyname);
}

template<typename T, typename V, V& (T::*member)()>
static inline void luameta_getter_member(lua_State* L, const LuaValue& value,
		const char* keyname) {
	lua_pushcfunction(L, (&luacpp_getmemberfn<T,V,member>));
	luameta_pop_getter(L, value, keyname);
}

template<typename T, typename V, V& (T::*member)()>
static inline void luameta_member(lua_State* L, const LuaValue& value,
		const char* keyname) {
	lua_pushcfunction(L, (&luacpp_setmemberfn<T,V,member>));
	luameta_pop_setter(L, value, keyname);
	luameta_getter_member<T, V, member>(L, value, keyname);
}

template<typename T, typename V, void (T::*member)(V v)>
static inline void luameta_setter_member(lua_State* L, const LuaValue& value,
		const char* keyname) {
	lua_pushcfunction(L, (&luacpp_setter<T,V,member>));
	luameta_pop_setter(L, value, keyname);
}

#endif /* LUACPP_METATABLE_H_ */
