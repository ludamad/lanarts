/*
 * luacpp.h:
 *  Wrap the SLB-based implementation of our pushes and gets.
 *	Include eg 'lua_timer' to have the overrides available
 */

#ifndef LUACPP_H_
#define LUACPP_H_

#include <SLB/PushGet.hpp>

template<typename T>
inline void luacpp_push(lua_State* L, const T& value) {
	SLB::push<T>(L, value);
}

template<typename T>
inline void luacpp_push(lua_State* L, T value) {
	SLB::push<T>(L, value);

}
template<typename T>
inline T luacpp_get(lua_State* L, int pos) {
	return SLB::get<T>(L, pos);
}

template<typename T>
inline void luacpp_get(T& value, lua_State* L, int pos) {
	value = SLB::get<T>(L, pos);
}


#endif /* LUACPP_H_ */
