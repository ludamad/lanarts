/*
 * members.h:
 *  Adapters for CPP member get/set as lua functions.
 *  For use with 'luameta.h'
 */

#ifndef LUAWRAP_MEMBERS_H_
#define LUAWRAP_MEMBERS_H_

#include <lua.hpp>

#include <luawrap/config.h>
#include <luawrap/luawrap.h>

namespace luawrap {
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

	template<typename T, typename V, V (T::*member)() const>
	static int getter(lua_State* L) {
		T& object = get<T&>(L, 1);
		push<V>(L, (object.*member)());
		return 1;
	}

	namespace _private {
		template<typename T, typename V>
		inline int getter(lua_State* L) {
			typedef V T::* M;
			M* member = (M*) lua_touserdata(L, lua_upvalueindex(1));
			T& object = get<T&>(L, 1);
			push<V>(L, object.*(*member));
			return 1;
		}
		template<typename T, typename V>
		inline int setter(lua_State* L) {
			typedef V T::* M;
			M* member = (M*) lua_touserdata(L, lua_upvalueindex(1));
			T& object = get<T&>(L, 1);
			// 2nd arg is 'key'
			get<V>(L, 3, object.*(*member));
			return 0;
		}
	}

}

namespace luawrap {
	template<typename LuaWrapper, typename T, typename V>
	void bind_getter(const LuaWrapper& field, V T::*member) {
		void* userdata = lua_newuserdata(field.luastate(), sizeof(member));
		*((V T::**) userdata) = member;

		lua_pushcclosure(field.luastate(), luawrap::_private::getter<T, V>, 1);
		field.pop();
	}

	template<typename LuaWrapper, typename T, typename V>
	void bind_setter(const LuaWrapper& field, V T::*member) {
		void* userdata = lua_newuserdata(field.luastate(), sizeof(member));
		*((V T::**) userdata) = member;

		lua_pushcclosure(field.luastate(), luawrap::_private::setter<T, V>, 1);
		field.pop();
	}
}

#endif /* LUAWRAP_MEMBERS_H_ */
