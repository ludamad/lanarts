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

extern "C" {
#include <lua/lua.h>
}

#include "luawrap.h"
#include "types.h"

namespace luawrap {
namespace _private {

/* Callbacks */
template<typename T>
class TypeImpl {
public:
	static void push(lua_State* L, const T& val) {
		CallbackStore<T>::callbacks.pushcallback(L, (const void*)&val);
	}
	static T get(lua_State* L, int idx) {
		T* valptr = NULL;
		CallbackStore<T>::callbacks.getcallback(L, idx, (void*)valptr);
		return *valptr;
	}
	static bool check(lua_State *L, int idx) {
		return CallbackStore<T>::callbacks.checkcallback(L, idx);
	}
};

/* Floating points */
template<>
class TypeImpl<double> {
public:
	static void push(lua_State* L, double val);
	static double get(lua_State* L, int idx);
	static bool check(lua_State *L, int idx);
};

template<>
class TypeImpl<float> : public TypeImpl<double> {
};

/* Integers */
template<>
class TypeImpl<long> {
public:
	static void push(lua_State* L, double val);

	static long get(lua_State* L, int idx);

	static bool check(lua_State* L, int idx);
};

template<>
class TypeImpl<unsigned long> : public TypeImpl<long> {
};

template<>
class TypeImpl<int> : public TypeImpl<long> {
};

template<>
class TypeImpl<unsigned int> : public TypeImpl<long> {
};

template<>
class TypeImpl<short> : public TypeImpl<long> {
};

template<>
class TypeImpl<unsigned short> : public TypeImpl<long> {
};

template<>
class TypeImpl<char> : public TypeImpl<long> {
};

template<>
class TypeImpl<unsigned char> : public TypeImpl<long> {
};

#ifndef LUAWRAP_NO_WRAP_LONGLONG

template<>
class TypeImpl<long long> : public TypeImpl<long> {
};

template<>
class TypeImpl<unsigned long long> : public TypeImpl<long> {
};
#endif

/* Strings */
template<>
class TypeImpl<std::string> {
public:
	static void push(lua_State* L, const std::string& val);
	static const char* get(lua_State* L, int idx);
	static bool check(lua_State *L, int idx);
};

template<>
class TypeImpl<const char*> : public TypeImpl<std::string> {
};
template<>
class TypeImpl<char*> : public TypeImpl<std::string> {
};

/* STL Containers */

#ifndef LUAWRAP_NO_WRAP_VECTOR

template<typename T>
class TypeImpl<std::vector<T> > {
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
