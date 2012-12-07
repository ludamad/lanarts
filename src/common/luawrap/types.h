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

template<typename T> class CallbackStore {
	static Callbacks callbacks;
};

template<typename T> Callbacks CallbackStore<T>::callbacks;
void install_callbacks(Callbacks& cb, push_t pushcallback, get_t getcallback,
		check_t checkcallback);

template<typename R, R (*func)(lua_State* L, int idx)>
void getcallback_adapter(lua_State* L, int idx, void* data) {
	new (data) R(func(L, idx));
}

}
}

namespace luawrap {

template<typename T, typename V>
void install_type(void (*pushfunc)(lua_State*, const V*),
		void (*getfunc)(lua_State*, int, V*),
		bool (*checkfunc)(lua_State*, int)) {
	using namespace _private;
	install_callbacks(CallbackStore<T>::callbacks, (push_t)pushfunc,
			(get_t)getfunc, (check_t)checkfunc);
}

}

#endif /* LUAWRAP_TYPES_H_ */
