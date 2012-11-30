/*
 * luacpp_wrap.h
 *  Helper macros for wrapping C++ types in lua
 */

#ifndef LUACPP_WRAP_H_
#define LUACPP_WRAP_H_

#include <SLB/Type.hpp>
#include <SLB/PushGet.hpp>

extern "C" {
#include <lua/lua.h>
}

#define LUACPP_GENERAL_WRAP(ParamTypes, SpecType, T, pushImpl, getImpl, checkImpl) \
	namespace SLB {	\
	namespace Private {	\
	template< ParamTypes > 	\
	struct Type<SpecType> { \
		typedef SpecType GetType; \
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

// We rely on the GCC semantics of extern template, so ignore if not in GCC:
#ifdef __GNUC__ 
#define LUACPP_TYPE_WRAP(T, pushf, getf, checkf) \
		extern template T SLB::get<T>(lua_State *L, int pos); \
		extern template bool SLB::check<T>(lua_State *L, int pos); \
		extern template void SLB::push<T>(lua_State *L, T v); \
		extern template bool SLB::check<const T &>(lua_State *L, int pos); \
		extern template void SLB::push<const T &>(lua_State *L, const T& v); \
		LUACPP_GENERAL_WRAP(,T, T, pushf, getf, checkf) \
		LUACPP_GENERAL_WRAP(/*Nothing*/, const T &, T, pushf, getf, checkf)
#else
#define LUACPP_TYPE_WRAP(T, pushf, getf, checkf) \
		LUACPP_GENERAL_WRAP(,T, T, pushf, getf, checkf) \
		LUACPP_GENERAL_WRAP(/*Nothing*/, const T &, T, pushf, getf, checkf)
#endif

#ifdef __GNUC__
#define LUACPP_TYPE_WRAP_IMPL(T) \
		template T SLB::get<T>(lua_State *L, int pos); \
		template bool SLB::check<T>(lua_State *L, int pos); \
		template void SLB::push<T>(lua_State *L, T v); \
		template bool SLB::check<const T &>(lua_State *L, int pos); \
		template void SLB::push<const T &>(lua_State *L, const T& v)
#else
#define LUACPP_TYPE_WRAP_IMPL(T)
#endif


//#define SLB_WRAP_VALUE_TYPE_IMPLCLAS(T, Impl) \
//		LUACPP_TYPE_WRAP(T, Impl::push, Impl::get, Impl::check)


#endif /* LUA_WRAP_UTIL_H_ */
