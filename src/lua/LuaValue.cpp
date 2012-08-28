#include <string>
#include <cstring>
#include <cstdlib>

#include <yaml-cpp/yaml.h>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "../serialize/SerializeBuffer.h"

#include "LuaValue.h"
#include "lua_api.h"
#include "lmarshal.h"

//YAML related helper functions

static bool nodeis(const YAML::Node& node, const char* str) {
	return (strcmp(node.Tag().c_str(), str) == 0);
}
void lua_push_yaml_node(lua_State* L, const YAML::Node& node) {
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
			lua_push_yaml_node(L, node[i]);
			lua_rawseti(L, table, i + 1);
		}
		break;
	case YAML::NodeType::Map:
		lua_newtable(L);
		table = lua_gettop(L);
		it = node.begin();
		for (; it != node.end(); ++it) {
			lua_push_yaml_node(L, it.first());
			lua_push_yaml_node(L, it.second());
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
			L(NULL), lua_expression(format_expression_string(expr)), refcount(
					1), empty(true) {
	}
	~LuaValueImpl() {
		if (L) {
			deinitialize(L);
		}
	}

	void deinitialize(lua_State* L) {
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_pushnil(L); /* push nil as value */
		lua_settable(L, LUA_REGISTRYINDEX);
		this->L = NULL;
	}

	void initialize(lua_State* L) {
		if (lua_expression.empty())
			return;
		this->L = L;
		empty = false;
		lua_pushlightuserdata(L, this); /* push address as key */
		luaL_dostring(L, lua_expression.c_str());
		lua_settable(L, LUA_REGISTRYINDEX);
		lua_expression.clear();
	}
	void table_initialize(lua_State* L) {
		empty = false;
		this->L = L;
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_newtable(L);
		lua_settable(L, LUA_REGISTRYINDEX);
	}

	void table_pop_value(lua_State* L, const char* key) {
		int value = lua_gettop(L);
		push(L); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushvalue(L, value); /*Clone value*/
		lua_setfield(L, tableind, key);
		/*Pop table and value*/
		lua_pop(L, 2);
	}

	void table_push_value(lua_State* L, const char* key) {
		push(L); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_getfield(L, tableind, key);
		lua_replace(L, tableind);
	}

	void table_set_function(lua_State* L, const char* key,
			lua_CFunction value) {
		push(L); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		/*Push the C function*/
		lua_pushcfunction(L, value);
		lua_setfield(L, tableind, key);
		/*Pop table*/
		lua_pop(L, 1);
	}
	void table_set_number(lua_State* L, const char* key, double value) {
		push(L); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_pushnumber(L, value); /*Push the number*/
		lua_setfield(L, tableind, key);
		/*Pop table*/
		lua_pop(L, 1);
	}
	void table_set_newtable(lua_State* L, const char* key) {
		push(L); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		lua_newtable(L);
		/*Push a new table*/
		lua_setfield(L, tableind, key);
		/*Pop table*/
		lua_pop(L, 1);
	}

	void table_set_yaml(lua_State* L, const char* key, const YAML::Node& root) {
		push(L); /*Get the associated lua table*/
		int tableind = lua_gettop(L);
		/*Push a YAML node as a lua value*/
		lua_push_yaml_node(L, root);
		lua_setfield(L, tableind, key);
		/*Pop table*/
		lua_pop(L, 1);
	}
	void push(lua_State* L) const {
		lua_pushlightuserdata(L, (void*)this); /* push address as key */
		lua_gettable(L, LUA_REGISTRYINDEX);
	}

	void pop(lua_State* L) {
		this->L = L;
		int valueidx = lua_gettop(L);
		lua_pushlightuserdata(L, this); /* push address as key */
		lua_pushvalue(L, valueidx); /*Clone value*/
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

	void serialize(lua_State* L, SerializeBuffer& serializer) {

		serializer.write(lua_expression);
		serializer.write(empty);
		push(L);
		lua_serialize(serializer, L, -1);
	}
	void deserialize(lua_State* L, SerializeBuffer& serializer) {
		serializer.read(lua_expression);
		serializer.read(empty);
		lua_deserialize(serializer, L);
		pop(L);
	}

private:
	lua_State* L;
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
	if (impl) {
		impl->deinitialize(L);
	}
}

void LuaValue::push(lua_State* L) const {
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
	if (impl) {
		impl->ref_count()++;}
	}

void LuaValue::operator =(const LuaValue & value) {
	deref(impl);
	impl = value.impl;
	if (impl)
		impl->ref_count()++;}

void LuaValue::table_get_number(lua_State *L, const char *key, double & value) {
	impl->table_push_value(L, key);
	value = lua_tonumber(L, -1);
	lua_pop(L, 1);
}

void LuaValue::table_copy(lua_State *L, LuaValue & othertable) {
}

void LuaValue::table_set_yaml(lua_State* L, const char *key,
		const YAML::Node& root) {
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

void lua_gameinst_callback(lua_State* L, LuaValue& value, GameInst* inst) {
	if (value.empty())
		return;
	value.push(L);
	lua_push_gameinst(L, inst);
	lua_call(L, 1, 0);
}

//Not used for now:
static void cached_gameinst_push(lua_State* L, LuaValue& cache,
		GameInst* inst) {
	if (cache.empty()) {
		cache.table_initialize(L);
		cache.push(L);
		int cacheind = lua_gettop(L);
		lua_pushlightuserdata(L, inst); /* push address as key */
		lua_gettable(L, cacheind);
		if (lua_isnil(L, -1)) {
			lua_push_gameinst(L, inst);
			lua_replace(L, cacheind);
		}
		lua_replace(L, cacheind);
	}
}

//Not used for now:
void lua_gameinst_cached_callback(lua_State* L, LuaValue& cache,
		LuaValue& value, GameInst* inst) {
	if (value.empty())
		return;
	value.push(L);
	cached_gameinst_push(L, cache, inst);
	lua_pop(L, 1);
	lua_call(L, 1, 0);
}

void LuaValue::serialize(lua_State* L, SerializeBuffer& serializer) {
	serializer.write_byte(impl != NULL);
	if (impl) {
		impl->serialize(L, serializer);
	}
}

void LuaValue::deserialize(lua_State* L, SerializeBuffer& serializer) {
	// Handle empty LuaValue's:
	int is_on;
	serializer.read_byte(is_on);
	if (!is_on) {
		delete impl;
		impl = NULL;
		return;
	}

	if (!impl) {
		impl = new LuaValueImpl();
	}
	impl->deserialize(L, serializer);
}

bool LuaValue::isnil(lua_State* L) {
	if (!impl) {
		return true;
	}
	impl->push(L);
	bool nilval = lua_isnil(L, -1);
	lua_pop(L, 1);
	return nilval;
}

