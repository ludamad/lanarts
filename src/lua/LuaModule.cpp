namespace YAML {
	class Node;
}
#include <cstring>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include <yaml-cpp/yaml.h>

void lua_registry_newtable(lua_State* L, void* addr){
    lua_pushlightuserdata(L, addr);  /* push address as key */
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);  
}

void lua_registry_push(lua_State* L, void* object){
    lua_pushlightuserdata(L, object);  /* push address as key */
    lua_gettable(L, LUA_REGISTRYINDEX);  
}

void lua_registry_erase(lua_State* L, void* object){
    lua_pushlightuserdata(L, object);  /* push address as key */
    lua_pushnil(L);  		       /* push nil as value */
    lua_settable(L, LUA_REGISTRYINDEX);  
}

class LuaModule {
public:
	LuaModule(lua_State* L) : L(L){
		lua_registry_push(L, this);
	}
	~LuaModule(){
		lua_registry_erase(L, this);
	}
	void set_function(const char* key, lua_CFunction value);
	void set_number(const char* key, double value);
	void set_newtable(const char* key);
	void set_yaml(const char* key, const YAML::Node* root);
	void pop(const char* key);
	void push(const char* key);
private:
	lua_State* L;
};



	
void LuaModule::pop(const char* key){
	int value = lua_gettop(L);
	lua_registry_push(L, this); /*Get the associated lua table*/	
	int tableind = lua_gettop(L);

	lua_pushstring(L, key);	/*Push the key*/
	lua_pushvalue(L, value); /*Clone value*/
	lua_gettable(L, value);
	lua_pop(L, 2); /*Pop table and value*/
}
	
void LuaModule::set_function(const char* key, lua_CFunction value){
	lua_registry_push(L, this); /*Get the associated lua table*/	
	int tableind = lua_gettop(L);
	lua_pushstring(L, key);	
	lua_pushcfunction(L, value); /*Push the C function*/
	lua_settable(L, tableind);
	lua_pop(L, 1); /*Pop table*/
}
void LuaModule::set_number(const char* key, double value){
	lua_registry_push(L, this); /*Get the associated lua table*/	
	int tableind = lua_gettop(L);
	lua_pushstring(L, key);	
	lua_pushnumber(L, value); /*Push the number*/
	lua_settable(L, tableind);
	lua_pop(L, 1); /*Pop table*/	
}
void LuaModule::set_newtable(const char* key){
	lua_registry_push(L, this); /*Get the associated lua table*/	
	int tableind = lua_gettop(L);
	lua_pushstring(L, key);	
	lua_newtable(L); /*Push a new table*/
	lua_settable(L, tableind);
	lua_pop(L, 1); /*Pop table*/	
}

static bool nodeis(const YAML::Node* node, const char* str){
	return (strcmp(node->Tag().c_str(), str) == 0);
}
static void push_yaml_node(lua_State* L, const YAML::Node* node){
	switch (node->Type()){
	case YAML::NodeType::Null:
		lua_pushnil(L);
		break;
	case YAML::NodeType::Scalar:
		if (nodeis(node, "str")){
			std::string str;
			*node >> str;
			lua_pushstring(L, str.c_str());
		} else if (nodeis(node, "float") || nodeis(node, "int")){
			double value;
			*node >> value;
			lua_pushnumber(L, value);
		}
		break;
	case YAML::NodeType::Sequence:
//		lua_pushtable
		break;
	case YAML::NodeType::Map:
		break;
	}
}
void LuaModule::set_yaml(const char* key, const YAML::Node* root){
	
}
