
#include "LuaValue.h"
#include <cstring>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include <yaml-cpp/yaml.h>

void LuaValue::initialize(){
    lua_pushlightuserdata(L, this);  /* push address as key */
    luaL_dostring(L, lua_expression.c_str());
	lua_settable(L, LUA_REGISTRYINDEX);
}
void LuaValue::deinitialize(){
    lua_pushlightuserdata(L, this);  /* push address as key */
    lua_pushnil(L);
	lua_settable(L, LUA_REGISTRYINDEX);
}

LuaValue::LuaValue(const std::string& expr)
	: lua_expression(expr) {
}

void LuaValue::push(){
    lua_pushlightuserdata(L, this);  /* push address as key */
    lua_gettable(L, LUA_REGISTRYINDEX);
}

LuaValue::~LuaValue() {
}

