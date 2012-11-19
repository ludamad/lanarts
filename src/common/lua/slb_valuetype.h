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

#define SLB_WRAP_VALUE_TYPE_IMPLCLAS(T, Impl) \
		SLB_WRAP_VALUE_TYPE(T, Impl::push, Impl::get, Impl::check)

#endif /* LUA_WRAP_UTIL_H_ */
