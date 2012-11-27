/*
 * lua_yaml.h:
 *  Converts YAML -> Lua
 */

#ifndef LUA_YAML_H_
#define LUA_YAML_H_

#include <common/lua/LuaValue.h>
#include <common/lua/luacpp.h>

namespace YAML {
class Node;
}

void lua_pushyaml(lua_State* L, const YAML::Node& node);
LuaValue lua_yaml(lua_State* L, const YAML::Node& node);

inline void luacpp_push(lua_State* L, const YAML::Node& node) {
	lua_pushyaml(L, node);
}

LuaValue parse_luacode(lua_State* L, const YAML::Node& node);
LuaValue parse_luacode(lua_State* L, const YAML::Node& node, const char* key);
LuaValue parse_luacode(lua_State* L, const YAML::Node& node, const char* key, const char* default_code);
#endif /* LUA_YAML_H_ */
