/*
 * types.h:
 *  Install types, implemented via a template-powered generic callback dispatch
 *  system.
 */

#ifndef LUAWRAP_TYPES_H_
#define LUAWRAP_TYPES_H_

#include <cstdlib>

struct lua_State;

namespace luawrap {
namespace _private {

typedef void (*push_t)(lua_State* L, const void* data);
typedef void (*get_t)(lua_State* L, int idx, void* data);
typedef bool (*check_t)(lua_State* L, int idx);

struct Callbacks {
	push_t pushcallback;
	get_t getcallback;
	check_t checkcallback;
	Callbacks(push_t pushcallback, get_t getcallback, check_t checkcallback);
	Callbacks();
};

// Generic callback storage
template<typename T> struct CallbackStore {
	static Callbacks callbacks;
};

template<typename T> Callbacks CallbackStore<T>::callbacks;

// Install callbacks in concrete location
void install_callbacks(Callbacks& cb, push_t pushcallback, get_t getcallback,
		check_t checkcallback);

// Adapt common push-format to desired format
template<typename T, typename Conv, void (*func)(lua_State* L, Conv)>
static inline void pushcallback_adapter(lua_State* L, const void* data) {
	func(L, *(const T*)data);
}

// Adapt common get-format to desired format
template<typename T, T (*func)(lua_State* L, int idx)>
static inline void getcallback_adapter(lua_State* L, int idx, void* data) {
	new (data) T(func(L, idx));
}

}
}

namespace luawrap {

template<typename T, void (*pushfunc)(lua_State*, const T&), T (*getfunc)(
		lua_State* L, int idx), bool (*checkfunc)(lua_State*, int)>
static inline void install_type() {
	using namespace _private;
	install_callbacks(CallbackStore<T>::callbacks,
			(push_t)pushcallback_adapter<T, const T&, pushfunc>,
			(get_t)getcallback_adapter<T, getfunc>, (check_t)checkfunc);
}

template<typename T, void (*pushfunc)(lua_State*, T), T (*getfunc)(lua_State* L,
		int idx), bool (*checkfunc)(lua_State*, int)>
static inline void install_type() {
	using namespace _private;
	install_callbacks(CallbackStore<T>::callbacks,
			(push_t)&pushcallback_adapter<T, T, pushfunc>,
			(get_t)&getcallback_adapter<T, getfunc>, (check_t)checkfunc);
}

}

#endif /* LUAWRAP_TYPES_H_ */
