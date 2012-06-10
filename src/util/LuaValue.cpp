#include "LuaValue.h"
#include <string>
#include <cstring>
#include <cstdlib>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "../lua/lua_api.h"

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
	std::string str;
	switch (node->Type()) {
	case YAML::NodeType::Null:
		lua_pushnil(L);
		break;
	case YAML::NodeType::Scalar:
		node->GetScalar(str);
		if (nodeis(node, "?")) {
			char* end;
			double value = strtod(str.c_str(), &end);
			size_t convchrs = (end - str.c_str());
			if (convchrs == str.size())
				lua_pushnumber(L, value);
			else
				lua_pushstring(L, str.c_str());
		} else {
			lua_pushstring(L, str.c_str());
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

static std::string format_expression_string(const std::string& str) {
	const char prefix[] = "return ";
	if (str.empty())
		return str;
	if (strncmp(str.c_str(), prefix, sizeof(prefix)) == 0)
		return str;
	return "return " + str;
}

class LuaValueImpl {
public:

	LuaValueImpl(const std::string& expr = std::string()) :
			lua_expression(format_expression_string(expr)), refcount(1), empty(
					true) {
	}

	void deinitialize(lua_State* L) {
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_pushnil(L);
		lua_settable(L, LUA_REGISTRYINDEX);
	}

	void initialize(lua_State* L) {
		if (lua_expression.empty())
			return;
		empty = false;
		lua_pushlightuserdata(L, this); /* push address as key */
		luaL_dostring(L, lua_expression.c_str());
		lua_settable(L, LUA_REGISTRYINDEX);
	}
	void table_initialize(lua_State* L) {
		empty = false;
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_newtable(L);
		lua_settable(L, LUA_REGISTRYINDEX);
	}

	void table_pop_value(lua_State* L, const char* key) {
		int value = lua_gettop(L);
		lua_registry_push(L, this); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key); /*Push the key*/
		lua_pushvalue(L, value); /*Clone value*/
		lua_settable(L, tableind);
		/*Pop table and value*/
		lua_pop(L, 2);
	}

	void table_push_value(lua_State* L, const char* key) {
		lua_registry_push(L, this); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key); /*Push the key*/
		lua_gettable(L, tableind);
//		lua_replace(L, tableind);
	}

	void table_set_function(lua_State* L, const char* key,
			lua_CFunction value) {
		lua_registry_push(L, this); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key);
		lua_pushcfunction(L, value);
		/*Push the C function*/
		lua_settable(L, tableind);
		lua_pop(L, 1);
		/*Pop table*/
	}
	void table_set_number(lua_State* L, const char* key, double value) {
		push(L); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key);
		lua_pushnumber(L, value); /*Push the number*/
		lua_settable(L, -3);
		/*Pop table*/
		lua_pop(L, 1);
	}
	void table_set_newtable(lua_State* L, const char* key) {
		lua_registry_push(L, this); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key);
		lua_newtable(L);
		/*Push a new table*/
		lua_settable(L, tableind);
		/*Pop table*/
		lua_pop(L, 1);
	}

	void table_set_yaml(lua_State* L, const char* key, const YAML::Node* root) {
		push(L); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushstring(L, key);
		push_yaml_node(L, root);
		lua_settable(L, tableind);
		/*Pop table*/
		lua_pop(L, 1);
	}
	void push(lua_State* L) {
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_gettable(L, LUA_REGISTRYINDEX);
	}

	void pop(lua_State* L) {
		int value = lua_gettop(L);
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_pushvalue(L, value); /*Clone value*/
		lua_settable(L, LUA_REGISTRYINDEX);

		lua_pop(L, 1);
		/*Pop value*/
	}

	size_t& ref_count() {
		return refcount;
	}

	bool is_empty() {
		return empty;
	}

private:
	std::string lua_expression;
	size_t refcount;
	bool empty;
};

static void deref(LuaValueImpl* impl) {
	if (impl && --impl->ref_count() == 0)
		delete impl;
}

LuaValue::LuaValue(const std::string & expr) {
	if (expr.empty())
		impl = NULL;
	else
		impl = new LuaValueImpl(expr);
}

LuaValue::LuaValue() {
	impl = NULL;
}

LuaValue::~LuaValue() {
	deref(impl);
}

void LuaValue::initialize(lua_State* L) {
	if (impl)
		impl->initialize(L);
}

void LuaValue::table_initialize(lua_State* L) {
	if (!impl)
		impl = new LuaValueImpl();
	impl->table_initialize(L);
}

void LuaValue::deinitialize(lua_State* L) {
	if (impl)
		impl->deinitialize(L);
}

void LuaValue::push(lua_State* L) {
	if (!impl)
		lua_pushnil(L);
	else
		impl->push(L);
}

void LuaValue::pop(lua_State* L) {
	if (!impl)
		impl = new LuaValueImpl();
	impl->pop(L);
}

void LuaValue::table_set_function(lua_State* L, const char *key,
		lua_CFunction value) {
	if (empty())
		table_initialize(L);
	impl->table_set_function(L, key, value);
}

void LuaValue::table_set_number(lua_State* L, const char *key, double value) {
	if (empty())
		table_initialize(L);
	impl->table_set_number(L, key, value);
}

void LuaValue::table_set_newtable(lua_State* L, const char *key) {
	if (empty())
		table_initialize(L);
	impl->table_set_newtable(L, key);
}

LuaValue::LuaValue(const LuaValue & value) {
	impl = value.impl;
	if (impl)
		impl->ref_count()++;}

void LuaValue::operator =(const LuaValue & value) {
	deref(impl);
	impl = value.impl;
	if (impl)
		impl->ref_count()++;}

void LuaValue::table_set_yaml(lua_State* L, const char *key,
		const YAML::Node *root) {
	if (empty())
		table_initialize(L);
	impl->table_set_yaml(L, key, root);
}

void LuaValue::table_pop_value(lua_State* L, const char *key) {
	if (empty())
		table_initialize(L);
	impl->table_pop_value(L, key);
}
void LuaValue::table_push_value(lua_State* L, const char *key) {
	if (empty())
		table_initialize(L);
	impl->table_push_value(L, key);
}

bool LuaValue::empty() {
	return impl == NULL || impl->is_empty();
}

void lua_gameinstcallback(lua_State* L, LuaValue& value, int id) {
	if (value.empty())
		return;
	value.push(L);
	lua_pushgameinst(L, id);
	lua_call(L, 1, 0);
}

