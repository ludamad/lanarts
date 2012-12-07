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

#ifndef LUAWRAP_FUNC_HELPER_H_
#define LUAWRAP_FUNC_HELPER_H_

namespace luawrap {
namespace _private {

template<typename T>
struct PushGetCheckWrap {
	static void push(lua_State* L, const T& val) {
		::luawrap::push<T>(L, val);
	}
	static T get(lua_State* L, int idx) {
		return ::luawrap::get<T>(L, idx);
	}
	static bool check(lua_State* L, int idx) {
		return ::luawrap::check<T>(L, idx);
	}
};

// Make sure const T& wrapped as T
template<typename T>
struct PushGetCheckWrap<const T&> : public PushGetCheckWrap<T> {
};
// Make sure T& wrapped as T*
template<typename T>
struct PushGetCheckWrap<T&> {
	static void push(lua_State* L, T& val) {
		::luawrap::push<T*>(L, &val);
	}
	static T& get(lua_State* L, int idx) {
		return *::luawrap::get<T*>(L, idx);
	}
	static bool check(lua_State* L, int idx) {
		return ::luawrap::check<T*>(L, idx);
	}
};

}
}

#endif /* LUAWRAP_FUNC_HELPER_H_ */
