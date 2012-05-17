#include "LuaValue.h"
#include <string>
#include <cstring>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include <yaml-cpp/yaml.h>

static void lua_registry_newtable(lua_State* L, void* addr) {
	lua_pushlightuserdata(L, addr); /* push address as key */
	lua_newtable(L);
	lua_settable(L, LUA_REGISTRYINDEX);
}

static void lua_registry_push(lua_State* L, void* object) {
	lua_pushlightuserdata(L, object); /* push address as key */
	lua_gettable(L, LUA_REGISTRYINDEX);
}

static void lua_registry_erase(lua_State* L, void* object) {
	lua_pushlightuserdata(L, object); /* push address as key */
	lua_pushnil(L); /* push nil as value */
	lua_settable(L, LUA_REGISTRYINDEX);
}

//YAML related helper functions

static bool nodeis(const YAML::Node* node, const char* str) {
	return (strcmp(node->Tag().c_str(), str) == 0);
}
static void push_yaml_node(lua_State* L, const YAML::Node* node) {
	int table;
	YAML::Iterator it;
	switch (node->Type()) {
	case YAML::NodeType::Null:
		lua_pushnil(L);
		break;
	case YAML::NodeType::Scalar:
		if (nodeis(node, "str")) {
			std::string str;
			*node >> str;
			lua_pushstring(L, str.c_str());
		} else if (nodeis(node, "float") || nodeis(node, "int")) {
			double value;
			*node >> value;
			lua_pushnumber(L, value);
		}
		break;
	case YAML::NodeType::Sequence:
		lua_newtable(L);
		table = lua_gettop(L);
		for (int i = 0; i < node->size(); i++) {
			push_yaml_node(L, &(*node)[i]);
			lua_rawseti(L, table, i + 1);
		}
		break;
	case YAML::NodeType::Map:
		lua_newtable(L);
		table = lua_gettop(L);
		it = node->begin();
		for (; it != node->end(); ++it) {
			push_yaml_node(L, &it.first());
			push_yaml_node(L, &it.second());
			lua_settable(L, table);
		}
		break;
	}
}

class LuaValueImpl {
public:

	LuaValueImpl(lua_State* L, const std::string& expr = std::string()) :
			lua_expression(expr) {
	}

	~LuaValueImpl() {
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_pushnil(L);
		lua_settable(L, LUA_REGISTRYINDEX);
	}

	void initialize() {
		if (lua_expression.empty())
			return;
		lua_pushlightuserdata(L, this); /* push address as key */
		luaL_dostring(L, lua_expression.c_str());
		double number = lua_tonumber(L, -1);
		lua_settable(L, LUA_REGISTRYINDEX);
	}

	void pop(const char* key) {
		int value = lua_gettop(L);
		lua_registry_push(L, this); /*Get the associated lua table*/
		int tableind = lua_gettop(L);

		lua_pushstring(L, key); /*Push the key*/
		lua_pushvalue(L, value); /*Clone value*/
		lua_settable(L, value);
		/*Pop table and value*/
		lua_pop(L, 2);
	}

	void set_function(const char* key, lua_CFunction value) {
		lua_registry_push(L, this); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key);
		lua_pushcfunction(L, value);
		/*Push the C function*/
		lua_settable(L, tableind);
		lua_pop(L, 1);
		/*Pop table*/
	}
	void set_number(const char* key, double value) {
		lua_registry_push(L, this); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key);
		lua_pushnumber(L, value); /*Push the number*/
		lua_settable(L, tableind);
		/*Pop table*/
		lua_pop(L, 1);
	}
	void set_newtable(const char* key) {
		lua_registry_push(L, this); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key);
		lua_newtable(L);
		/*Push a new table*/
		lua_settable(L, tableind);
		/*Pop table*/
		lua_pop(L, 1);
	}

	void set_yaml(const char* key, const YAML::Node* root) {
		lua_registry_push(L, this); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key);
		push_yaml_node(L, root);
		/*Pop table*/
		lua_pop(L, 1);
	}
	void push() {
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_gettable(L, LUA_REGISTRYINDEX);
	}

	void pop() {
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_gettable(L, LUA_REGISTRYINDEX);
	}

private:
	std::string lua_expression;
	lua_State* L;
};

LuaValue::LuaValue(lua_State* L, const std::string & expr) {
	impl = new LuaValueImpl(L, expr);
}

LuaValue::LuaValue(lua_State* L) {
	impl = NULL;
}

LuaValue::~LuaValue() {
	delete impl;
}

void LuaValue::initialize() {
	if (!impl)
		impl = new LuaValueImpl(L);
	impl->initialize();
}

void LuaValue::push() {
	impl->push();
}

void LuaValue::pop() {
	impl->pop();
}

void LuaValue::set_function(const char *key, lua_CFunction value) {
	if (!impl)
		impl = new LuaValueImpl(L);
	impl->set_function(key, value);
}

void LuaValue::set_number(const char *key, double value) {
	if (!impl)
		impl = new LuaValueImpl(L);
	impl->set_number(key, value);
}

void LuaValue::set_newtable(const char *key) {
	if (!impl)
		impl = new LuaValueImpl(L);
	impl->set_newtable(key);
}

void LuaValue::set_yaml(const char *key, const YAML::Node *root) {
	if (!impl)
		impl = new LuaValueImpl(L);
	impl->set_yaml(key, root);
}

