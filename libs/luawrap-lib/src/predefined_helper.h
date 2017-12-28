/*
 * predefined_helper.h:
 *  Predefined wrappers for get/check/push. These types cannot be customized (other
 *  than by manually editing this file).
 *  Use LUAWRAP_NO_WRAP_VECTOR to not wrap vector.
 */

#ifndef LUAWRAP_PREDEFINED_HELPER_H_
#define LUAWRAP_PREDEFINED_HELPER_H_

#include <string>

#ifndef LUAWRAP_NO_WRAP_VECTOR
#include <vector> // Only include if need-be
#endif

#include <lua.hpp>

#include <type_traits>
#include <luawrap/LuaValue.h>
#include <luawrap/types.h>

#include "lambda_util.h"

#include "pushget_helper.h"

namespace luawrap {

	template<typename T>
	inline void push(lua_State* L, const T& val);

	template<typename T>
	inline typename _private::PushGetCheckWrap<T>::RetType get(lua_State* L, int idx);

	template<typename T>
	inline bool check(lua_State* L, int idx);

	namespace _private {

		/* Everything not predefined: Callback fallback */
		template<typename T, typename Dummy>
		class TypeImpl {
		public:
			static void push(lua_State* L, const T& val) {
				CallbackStore<T>::callbacks.pushcallback(L, (const void*)&val);
			}
			static T get(lua_State* L, int idx) {
				char valbuffer[sizeof(T)];
				CallbackStore<T>::callbacks.getcallback(L, idx,
						(void*)valbuffer);
				return *(T*)valbuffer;
			}
			static bool check(lua_State *L, int idx) {
				return CallbackStore<T>::callbacks.checkcallback(L, idx);
			}
		};

		/* Lua value -> matches any value! */
		template<>
		class TypeImpl<LuaValue, bool> {
		public:
			static void push(lua_State* L, const LuaValue& val);
			static LuaValue get(lua_State* L, int idx);
			static bool check(lua_State *L, int idx);
		};

		/* Lua stack value -> matches any value! */
		template<>
		class TypeImpl<LuaStackValue, bool> {
		public:
			static void push(lua_State* L, const LuaStackValue& val);
			static LuaStackValue get(lua_State* L, int idx);
			static bool check(lua_State *L, int idx);
		};

		/* Lua field value -> matches any value! */
		template<>
		class TypeImpl<LuaField, bool> {
		public:
			static void push(lua_State* L, const LuaField& val);
			static LuaField get(lua_State* L, int idx);
			static bool check(lua_State *L, int idx);
		};

		/* Standard lua C function */
		template<>
		class TypeImpl<lua_CFunction, bool> {
		public:
			static void push(lua_State* L, lua_CFunction val);
			static lua_CFunction get(lua_State* L, int idx);
			static bool check(lua_State *L, int idx);
		};

		/* Boolean */
		template<>
		class TypeImpl<bool, bool> {
		public:
			static void push(lua_State* L, bool val);
			static bool get(lua_State* L, int idx);
			static bool check(lua_State *L, int idx);
		};

		/* Floating points */
		template<>
		class TypeImpl<double, bool> {
		public:
			static void push(lua_State* L, double val);
			static double get(lua_State* L, int idx);
			static bool check(lua_State *L, int idx);
		};

		template<>
		class TypeImpl<float, bool> : public TypeImpl<double, bool> {
		};

		/* Integers */
		template<>
		class TypeImpl<long, bool> {
		public:
			static void push(lua_State* L, double val);

			static long get(lua_State* L, int idx);

			static bool check(lua_State* L, int idx);
		};

		template<>
		class TypeImpl<unsigned long, bool> : public TypeImpl<long, bool> {
		};

		template<>
		class TypeImpl<int, bool> : public TypeImpl<long, bool> {
		};

		template<>
		class TypeImpl<unsigned int, bool> : public TypeImpl<long, bool> {
		};

		template<>
		class TypeImpl<short, bool> : public TypeImpl<long, bool> {
		};

		template<>
		class TypeImpl<unsigned short, bool> : public TypeImpl<long, bool> {
		};

		template<>
		class TypeImpl<char, bool> : public TypeImpl<long, bool> {
		};

		template<>
		class TypeImpl<unsigned char, bool> : public TypeImpl<long, bool> {
		};

#ifndef LUAWRAP_NO_WRAP_LONGLONG

		template<>
		class TypeImpl<long long, bool> : public TypeImpl<long, bool> {
		};

		template<>
		class TypeImpl<unsigned long long, bool> : public TypeImpl<long, bool> {
		};
#endif

		/* Strings */
		template<>
		class TypeImpl<std::string, bool> {
		public:
			static void push(lua_State* L, const std::string& val);
			static void push(lua_State* L, const char* val);
			static const char* get(lua_State* L, int idx);
			static bool check(lua_State *L, int idx);
		};

		template<>
		class TypeImpl<const char*, bool> : public TypeImpl<std::string, bool> {
		};
		template<>
		class TypeImpl<char*, bool> : public TypeImpl<std::string, bool> {
		};

		template<typename T, typename = decltype(&T::operator())>
		using enable_lambda = T;

		/* C++ Lambdas */
		template<typename T>
		class TypeImpl<enable_lambda<T>, bool> {
		public:
			static void push(lua_State* L, const T& data) {
                                lua_push_unsafe_closure(L, data);
			}
			static LuaValue get(lua_State* L, int idx) {
				return LuaValue();
			}
			static bool check(lua_State *L, int idx) {
				return true; // Take on faith for now
			}
		};

		/* STL Containers */

#ifndef LUAWRAP_NO_WRAP_VECTOR
		template<typename T>
		class TypeImpl<std::vector<T>, bool> {
		public:
			static void push(lua_State* L, const std::vector<T>& value) {
				lua_newtable(L);
				int idx = lua_gettop(L);
				for (int i = 0; i < value.size(); i++) {
					luawrap::push<T>(L, value[i]);
					lua_rawseti(L, idx, i + 1);
				}
			}

			static std::vector<T> get(lua_State* L, int idx) {
				std::vector<T> value;

				int size = lua_objlen(L, idx);
				for (int i = 0; i < size; i++) {
					lua_rawgeti(L, idx, i + 1);
					value.push_back(luawrap::get<T>(L, -1));

					lua_pop(L, 1);
				}
				return value;
			}

			static bool check(lua_State *L, int idx) {
				int size = lua_objlen(L, idx);
				bool valid = true;

				for (int i = 0; i < size && valid; i++) {
					lua_rawgeti(L, idx, i + 1);
					if (!luawrap::check<T>(L, -1)) {
						valid = false;
					}

					lua_pop(L, 1);
				}
				return valid;
			}
		};
#endif

	}
}

#endif /* LUAWRAP_PREDEFINED_HELPER_H_ */
