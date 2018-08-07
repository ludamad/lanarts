#ifndef DATA_LUA_UTIL
#define DATA_LUA_UTIL

#include <new>

#include "lambda_util.h"
#include <luawrap/luawrap.h>
#include "lanarts_defines.h"

template<typename LuaWrapper, typename T, typename ...Args>
inline T lmethod_opt_call(T _default, const LuaWrapper& wrapper, const char* key, const Args... args) {
    LuaField field = wrapper[key];
    if (field.isnil()) {
        return _default;
    }
    field.push();
    return luawrap::call<T>(wrapper.luastate(), wrapper, args...);
}

template<typename T, typename LuaWrapper, typename ...Args>
inline T lmethod_call(const LuaWrapper& wrapper, const char* key, const Args... args) {
    LuaField field = wrapper[key];
    field.push();
    return luawrap::call<T>(wrapper.luastate(), wrapper, args...);
}

template<typename LuaWrapper, typename ...Args>
inline void lmethod_opt_call(const LuaWrapper& wrapper, const char* key, const Args... args) {
    LuaField field = wrapper[key];
    if (field.isnil()) {
        return;
    }
    field.push();
    luawrap::call<void>(wrapper.luastate(), wrapper, args...);
}

template<typename ...Args>
inline void lcall(const LuaValue& field, const Args... args) {
    if (field.empty() || field.isnil()) {
        return;
    }
    field.push();
    luawrap::call<void>(field.luastate(), args...);
}

template<typename T, typename ...Args>
inline T lcall(const LuaValue& field, const Args... args) {
    if (field.empty() || field.isnil()) {
        return T();
    }
    field.push();
    return luawrap::call<T>(field.luastate(), args...);
}

template<typename ...Args>
inline void lcall(const LuaField& field, const Args... args) {
    if (field.isnil()) {
        return;
    }
    field.push();
    luawrap::call<void>(field.luastate(), args...);
}

template<typename T, typename ...Args>
inline T lcall(const LuaField& field, const Args... args) {
    if (field.isnil()) {
        return T();
    }
    field.push();
    return luawrap::call<T>(field.luastate(), args...);
}

template<typename ...Args>
inline void lmeth(const LuaValue& field, const char* key, const Args... args) {
    if (field[key].isnil()) {
        return;
    }
    field[key].push();
    luawrap::call<void>(field.luastate(), field, args...);
}

template<typename T, typename ...Args>
inline T lcall_def(T _default, const LuaValue& field, const Args... args) {
    if (field.empty() || field.isnil()) {
        return _default;
    }
    field.push();
    return luawrap::call<T>(field.luastate(), args...);
}

#endif