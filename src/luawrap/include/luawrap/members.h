/*
 * members.h:
 *  Adapters for CPP member get/set as lua functions.
 *  For use with 'luameta.h'
 */

#ifndef LUAWRAP_MEMBERS_H_
#define LUAWRAP_MEMBERS_H_

#include <luawrap/config.h>
#include <luawrap/luawrap.h>

namespace luawrap {

	template<typename T, typename V, V T::* member>
	static int setter(lua_State* L) {
		T& object = get<T&>(L, 1);
		// 2nd arg is 'key'
		check<V>(L, 3);
		get<V>(L, 3, object.*member);
		return 0;
	}

	template<typename T, typename V, V T::* member>
	static int getter(lua_State* L) {
		T& object = get<T&>(L, 1);
		push<V>(L, object.*member);
		return 1;
	}

	template<typename T, typename V, V& (T::*member)()>
	static int setter(lua_State* L) {
		T& object = get<T&>(L, 1);
		// 2nd arg is 'key'
		get<V>(L, 3, (object.*member)());
		return 0;
	}

	template<typename T, typename V, void (T::*member)(V)>
	static int setter(lua_State* L) {
		T& object = get<T&>(L, 1);
		// 2nd arg is 'key'
		(object.*member)(get<V>(L, 3));
		return 0;
	}

	template<typename T, typename V, V& (T::*member)()>
	static int getter(lua_State* L) {
		T& object = get<T&>(L, 1);
		push<V>(L, (object.*member)());
		return 1;
	}

}

#endif /* LUAWRAP_MEMBERS_H_ */
