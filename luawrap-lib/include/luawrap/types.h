/*
 * types.h:
 *  Install types, implemented via a template-powered generic callback dispatch
 *  system.
 */

#ifndef LUAWRAP_TYPES_H_
#define LUAWRAP_TYPES_H_

#include <string>

#include <typeinfo>
#include <lua.hpp>

#include <new>
#include <cstdlib>

#include <luawrap/config.h>
#include <luawrap/luameta.h>

struct lua_State;

namespace luawrap {
	namespace _private {
		/* Attempt to make the output of typeinfo readable for nicer errors, if possible. */
		std::string demangle_typename(const char* name);
	}
}

namespace luawrap {
	namespace _private {

		typedef void (*push_t)(lua_State* L, const void* data);
		typedef void (*get_t)(lua_State* L, int idx, void* data);
		typedef bool (*check_t)(lua_State* L, int idx);

		struct Callbacks {
			push_t pushcallback;
			get_t getcallback;
			check_t checkcallback;
			Callbacks(push_t pushcallback, get_t getcallback,
					check_t checkcallback);
			Callbacks();
		};

		// Generic callback storage
		template<typename T> struct CallbackStore {
			static Callbacks callbacks;
		};

		template<typename T> Callbacks CallbackStore<T>::callbacks;

		// Install callbacks in concrete location
		void install_callbacks(Callbacks& cb, push_t pushcallback,
				get_t getcallback, check_t checkcallback);

		// Adapt common push-format to desired format
		template<typename T, typename Conv, void (*func)(lua_State* L, Conv)>
		static inline void pushcallback_adapter(lua_State* L,
				const void* data) {
			func(L, *(const T*)data);
		}

		// Adapt common get-format to desired format
		template<typename T, T (*func)(lua_State* L, int idx)>
		static inline void getcallback_adapter(lua_State* L, int idx,
				void* data) {
			new (data) T(func(L, idx));
		}

	}

	template<typename T, void (*pushfunc)(lua_State*, const T&), T (*getfunc)(
			lua_State* L, int idx), bool (*checkfunc)(lua_State*, int)>
	static inline void install_type() {
		using namespace _private;

		install_callbacks(CallbackStore<T>::callbacks,
				(push_t)pushcallback_adapter<T, const T&, pushfunc>,
				(get_t)getcallback_adapter<T, getfunc>, (check_t)checkfunc);
	}

	template<typename T, void (*pushfunc)(lua_State*, T), T (*getfunc)(
			lua_State* L, int idx), bool (*checkfunc)(lua_State*, int)>
	static inline void install_type() {
		using namespace _private;

		install_callbacks(CallbackStore<T>::callbacks,
				(push_t)&pushcallback_adapter<T, T, pushfunc>,
				(get_t)&getcallback_adapter<T, getfunc>, (check_t)checkfunc);
	}

	//Push-only
	template<typename T, void (*pushfunc)(lua_State*, T)>
	static inline void install_type() {
		using namespace _private;
		install_callbacks(CallbackStore<T>::callbacks,
				(push_t)&pushcallback_adapter<T, T, pushfunc>, NULL, NULL);
	}

	template<typename T, void (*pushfunc)(lua_State*, const T&)>
	static inline void install_type() {
		using namespace _private;
		install_callbacks(CallbackStore<T>::callbacks,
				(push_t)&pushcallback_adapter<T, const T&, pushfunc>, NULL,
				NULL);
	}

}

/* luameta wrapping helpers*/

namespace luawrap {

	namespace _private {

		template<typename T>
		LuaValue plaindata_initializer(lua_State* L) {
			LuaValue meta = luameta_new(L, typeid(T).name());
			luameta_gc<T>(meta);
			return meta;
		}

		bool check_is_luametatype(lua_State* L, int idx,
				luameta_initializer meta);

		// Adapt common get-format to desired format
		template<luameta_initializer meta>
		static inline bool check_userdata(lua_State* L, int idx) {
			return check_is_luametatype(L, idx, meta);
		}

		// Adapt common get-format to desired format
		template<typename T>
		static inline void get_userdata(lua_State* L, int idx, void* data) {
			new (data) T(*(T*)lua_touserdata(L, idx));
		}

		template<typename T>
		static inline void get_userdata_ptr(lua_State* L, int idx, void* data) {
			*(void**)data = lua_touserdata(L, idx);
		}

		template<typename T, luameta_initializer meta>
		inline void push_userdata(lua_State* L, const T& val) {
			void* valptr = luameta_newuserdata(L, meta, sizeof(T));
			new (valptr) T(val);
		}
	}

	template<typename T, void (*pushfunc)(lua_State*, const T&),
			luameta_initializer meta>
	static inline void install_userdata_type() {
		using namespace _private;

		install_callbacks(CallbackStore<T>::callbacks,
				&pushcallback_adapter<T, const T&, pushfunc>, &get_userdata<T>,
				&check_userdata<meta>);

		install_callbacks(CallbackStore<T*>::callbacks, NULL,
				&get_userdata_ptr<T>, &check_userdata<meta>);
	}

	template<typename T, luameta_initializer meta>
	static inline void install_userdata_type() {
		using namespace _private;
		install_userdata_type<T, &push_userdata<T, meta>, meta>();
	}

	template<typename T>
	static inline void install_plaindata_type() {
		using namespace _private;
		install_userdata_type<T,
				&push_userdata<T, plaindata_initializer<T> >,
				plaindata_initializer<T> >();
	}

}

/* Analogous type helper*/
namespace luawrap {

	namespace _private {
		template<typename T>
		class PushGetCheckWrap;
	}

	template<typename T> void push(lua_State* L, const T& val);

	template<typename T>
	inline typename _private::PushGetCheckWrap<T>::RetType get(lua_State* L,
			int idx);

	template<typename T>
	inline bool check(lua_State* L, int idx);

	namespace _private {
		template<typename To, typename From>
		inline To get_casted(lua_State* L, int idx) {
			return (To)luawrap::get<From>(L, idx);
		}

		template<typename To, typename From>
		inline To get_dynamic_casted(lua_State* L, int idx) {
			return dynamic_cast<To>(luawrap::get<From>(L, idx));
		}

		template<typename To, typename From>
		inline void push_casted(lua_State* L, const To& val) {
			luawrap::push<From>(L, (From)val);
		}

		template<typename To, typename From>
		inline void push_dynamic_casted(lua_State* L, const To& val) {
			luawrap::push<From>(L, dynamic_cast<From>(val));
		}
	}

	template<typename To, typename From>
	static inline void install_casted_type() {
		using namespace _private;

		install_type<To, &push_casted<To, From>, &get_casted<To, From>,
				&luawrap::check<From> >();
	}

	template<typename To, typename From>
	static inline void install_dynamic_casted_type() {
		using namespace _private;

		install_type<To, &push_dynamic_casted<To, From>, &get_dynamic_casted<To, From>,
				&luawrap::check<From> >();
	}
}
#endif /* LUAWRAP_TYPES_H_ */
