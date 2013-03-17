/*
 * func_helper.h:
 *  Helper for correct argument forwarding:
 *  	T& -> T*
 *  	const T& -> T
 *  While const T& could be represented by T*, it is kept a value type for
 *  simplicity reasons. When pointer semantics are desired (for copying efficiency)
 *  crafted forwarding functions (ie convert arguments and do desired operation)
 *  should be created.
 */

#ifndef LUAWRAP_PUSHGET_HELPER_H_
#define LUAWRAP_PUSHGET_HELPER_H_

#include <lua.hpp>

namespace luawrap {
	namespace _private {

		template<class C>
		struct TypeImpl;

		template<typename T>
		struct PushGetCheckWrap {
			typedef T RetType;
			static void push(lua_State* L, const T& val) {
				luawrap::_private::TypeImpl<T>::push(L, val);
			}
			static T get(lua_State* L, int idx) {
				return TypeImpl<T>::get(L, idx);
			}
			static bool check(lua_State* L, int idx) {
				return TypeImpl<T>::check(L, idx);
			}
		};

		template<>
		struct PushGetCheckWrap<void> {
			typedef void RetType;
			static void get(lua_State* L, int idx) {
			}
			static bool check(lua_State* L, int idx) {
				return true;
			}
		};

		// Make sure const T& wrapped as T
		template<typename T>
		struct PushGetCheckWrap<const T&> : public PushGetCheckWrap<T> {
		};

		// Make sure T& wrapped as T*
		template<typename T>
		struct PushGetCheckWrap<T&> {
			typedef T& RetType;
			static void push(lua_State* L, T& val) {
				TypeImpl<T*>::push(L, val);
			}
			static T& get(lua_State* L, int idx) {
				return *TypeImpl<T*>::get(L, idx);
			}
			static bool check(lua_State* L, int idx) {
				return TypeImpl<T*>::check(L, idx);
			}
		};

		// Make sure const T& wrapped as T
		template<typename T>
		struct PushGetCheckWrap<const T*> : public PushGetCheckWrap<T*> {
			typedef const T* RetType;
			static void push(lua_State* L, const T* val) {
				luawrap::_private::TypeImpl<T*>::push(L, (T*)val);

			}
			static const T* get(lua_State* L, int idx) {
				return (const T*)TypeImpl<T*>::get(L, idx);
			}
		};

		// Make sure const char[] is treated as char*
		template<int N>
		struct PushGetCheckWrap<const char[N]> : public PushGetCheckWrap<
				const char*> {
		};

		// Make sure const char[] is treated as char*
		template<int N>
		struct PushGetCheckWrap<char[N]> : public PushGetCheckWrap<const char*> {
		};

	}
}

#endif /* LUAWRAP_PUSHGET_HELPER_H_ */
