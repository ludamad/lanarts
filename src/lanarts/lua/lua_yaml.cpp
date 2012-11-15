/*
 * lua_yaml.cpp:
 *  Converts YAML -> Lua
 */

#include <string>
#include <cstring>
#include <cstdlib>

#include <yaml-cpp/yaml.h>

extern "C" {
#include <lua/lua.h>
}

#include "lua_yaml.h"

//YAML related helper functions

static bool nodeis(const YAML::Node& node, const char* str) {
	return (strcmp(node.Tag().c_str(), str) == 0);
}
void lua_pushyaml(lua_State* L, const YAML::Node& node) {
	int table;
	YAML::Iterator it;
	std::string str;
	switch (node.Type()) {
	case YAML::NodeType::Null:
		lua_pushnil(L);
		break;
	case YAML::NodeType::Scalar:
		node.GetScalar(str);
		if (nodeis(node, "?")) {
			if (str == "yes") {
				lua_pushboolean(L, true);
			} else if (str == "no") {
				lua_pushboolean(L, false);
			} else { //Else its a number or a string
				char* end = NULL;
				double value = strtod(str.c_str(), &end);
				size_t convchrs = (end - str.c_str());
				if (convchrs == str.size())
					lua_pushnumber(L, value);
				else
					lua_pushstring(L, str.c_str());
			}
		} else {
			lua_pushstring(L, str.c_str());
		}
		break;
	case YAML::NodeType::Sequence:
		lua_newtable(L);
		table = lua_gettop(L);
		for (int i = 0; i < node.size(); i++) {
			lua_pushyaml(L, node[i]);
			lua_rawseti(L, table, i + 1);
		}
		break;
	case YAML::NodeType::Map:
		lua_newtable(L);
		table = lua_gettop(L);
		it = node.begin();
		for (; it != node.end(); ++it) {
			lua_pushyaml(L, it.first());
			lua_pushyaml(L, it.second());
			lua_settable(L, table);
		}
		break;
	}
}

LuaValue lua_yaml(lua_State *L, const YAML::Node & node) {
	LuaValue ret;
	lua_pushyaml(L, node);
	ret.pop(L);
	return ret;
}

