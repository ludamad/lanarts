/*
 * SLB_luacpp_hack.h:
 *  XXX: This is added because SLB was being a real pain when I wanted to
 *  overload the push/get/check methods.
 *  This adds
 */

#ifndef SLB_LUACPP_HACK_H_
#define SLB_LUACPP_HACK_H_

struct lua_State;

namespace SLB {
namespace Private {
template<class C>
struct Type;
}
}

template<typename T>
inline void luacpp_push(lua_State* L, const T& value) {
	SLB::Private::Type<T>::push(L, value);
}

template<typename T>
inline void luacpp_get(lua_State* L, int idx, T& value) {
	value = SLB::Private::Type<T>::get(L, idx);
}

template<typename T>
inline bool luacpp_checked_get(lua_State* L, int idx, T& value) {
	if (SLB::Private::Type<T>::check(L, idx)) {
		value = SLB::Private::Type<T>::get(L, idx);
		return true;
	}
	return false;
}

#endif /* SLB_LUACPP_HACK_H_ */
