#ifndef DATA_LAMBDA_UTIL
#define DATA_LAMBDA_UTIL

#include <functional>
#include "luawrap/luawrap.h"

// 0-cost temporary lambda binding for lua -> C++
template <typename Function>
struct TempFunctionStorer {
    static Function& get() {
        static char __data[sizeof(Function)];
        return *(Function*) __data;
    }
    static void reset() {
        get().~Function();
    }

    static void set(const Function& f) {
        new (&get()) Function(f);
    };
};

// START HAND WRITTEN LUA CLOSURE WRAPPERS
template <typename Function>
inline int _lua_lambda_wrap(lua_State* L) {
    auto& f = TempFunctionStorer<Function>::get();
    luawrap::push(L, f());
    return 1;
}
template <typename Function, typename Arg1>
inline int _lua_lambda_wrap(lua_State* L) {
    auto& f = TempFunctionStorer<Function>::get();
    luawrap::push(L, f(luawrap::get<Arg1>(L, 1)));
    return 1;
}

template <typename Function, typename Arg1, typename Arg2>
inline int _lua_lambda_wrap(lua_State* L) {
    auto& f = TempFunctionStorer<Function>::get();
    luawrap::push(L, f(luawrap::get<Arg1>(L, 1), luawrap::get<Arg2>(L, 2)));
    return 1;
}

template <typename Function, typename Arg1, typename Arg2, typename Arg3>
inline int _lua_lambda_wrap(lua_State* L) {
    auto& f = TempFunctionStorer<Function>::get();
    luawrap::push(L, f(luawrap::get<Arg1>(L, 1), luawrap::get<Arg2>(L, 2), luawrap::get<Arg3>(L, 3)));
    return 1;
}

template <typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
inline int _lua_lambda_wrap(lua_State* L) {
    auto& f = TempFunctionStorer<Function>::get();
    luawrap::push(L, f(luawrap::get<Arg1>(L, 1), luawrap::get<Arg2>(L, 2), luawrap::get<Arg3>(L, 3), luawrap::get<Arg3>(L, 4)));
    return 1;
}
// END HAND WRITTEN

template<class T>
struct function_traits : function_traits<decltype(&T::operator())> {
    typedef T Type;
};

// partial specialization for function type
template<class R, class... Args>
struct function_traits<R(Args...)> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;
};

// partial specialization for function pointer
template<class R, class... Args>
struct function_traits<R (*)(Args...)> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;

    template<typename F>
    static void push(lua_State* L, const F& f) {
        auto* cfunc = &(_lua_lambda_wrap<F, Args...>);
        lua_pushcfunction(L, cfunc);
    }
};

// partial specialization for std::function
template<class R, class... Args>
    struct function_traits<std::function<R(Args...)>> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;

    // Push is TODO
};

// partial specialization for pointer-to-member-function (i.e., operator()'s)
template<class T, class R, class... Args>
struct function_traits<R (T::*)(Args...)> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;

    template<typename F>
    static void push(lua_State* L, const F& f) {
        auto* cfunc = &(_lua_lambda_wrap<F, Args...>);
        lua_pushcfunction(L, cfunc);
    }
};

template<class T, class R, class... Args>
struct function_traits<R (T::*)(Args...) const> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;

    template<typename F>
    static void push(lua_State* L, const F& f) {
        auto* cfunc = &(_lua_lambda_wrap<F, Args...>);
        lua_pushcfunction(L, cfunc);
    }
};

template <typename F>
inline void lua_push_unsafe_closure(lua_State* L, const F& f) {
    static bool was_set = false;
    if (was_set) {
        TempFunctionStorer<F>::reset();
    }
    function_traits<F>::push(L, f);
    TempFunctionStorer<F>::set(f);
    was_set = true;
}

#endif