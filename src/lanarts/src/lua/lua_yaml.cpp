/*
 * lua_yaml.cpp:
 *  Converts YAML -> Lua
 */

#include <string>
#include <cstring>
#include <cstdlib>

#include <luawrap/types.h>

#include <lua.hpp>

#include <yaml-cpp/yaml.h>

#include "../lanarts_defines.h"
#include "lua_yaml.h"

//YAML related helper functions

// Calls install_type:
struct __LuaYAMLInitializer {
	__LuaYAMLInitializer() {
		luawrap::install_type<YAML::Node, lua_pushyaml>();
	}
} __initializer;

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
	lua_pushyaml(L, node);
	return LuaValue::pop_value(L);
}

static std::string format_expression_string(const char* str) {
	const char prefix[] = "return ";
	if (strncmp(str, prefix, sizeof(prefix)) == 0)
		return str;
	return std::string("return ") + str;
}

static LuaValue parse_luacode(lua_State* L, const char* code) {
	perf_timer_begin(FUNCNAME);

	std::string retcode = format_expression_string(code);

	int ntop = lua_gettop(L);
	if (luaL_loadstring(L, retcode.c_str())) {
		printf("Error while parsing lua expression:\n%s\nFor expression:\n%s",
				lua_tostring(L, -1), retcode.c_str());
		exit(0);
	}
	LANARTS_ASSERT(lua_gettop(L) - ntop == 1);

	perf_timer_end(FUNCNAME);
	return LuaValue::pop_value(L);
}

LuaValue parse_luacode(lua_State* L, const YAML::Node& node) {
	std::string code;
	node.GetScalar(code);
	return parse_luacode(L, code.c_str());
}

LuaValue parse_luacode(lua_State *L, const YAML::Node& node, const char* key) {
	const YAML::Node* child = node.FindValue(key);
	if (child) {
		return parse_luacode(L, *child);
	}
	return LuaValue();
}

LuaValue parse_luacode(lua_State *L, const YAML::Node& node, const char* key,
		const char* default_code) {
	LuaValue value = parse_luacode(L, node, key);
	if (value.empty()) {
		return parse_luacode(L, default_code);
	}
	return value;
}

