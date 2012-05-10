#if 0

namespace YAML {
	class Node;
}

extern "C" {
	#include <lua/lua.h>
}

void lua_registry_newtable(lua_state* L, void* addr){
    lua_pushlightuserdata(L, addr);  /* push address as key */
    lua_pushnewtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);  
}

void lua_registry_push(lua_state* L, void* object){
    lua_pushlightuserdata(L, object);  /* push address as key */
    lua_gettable(L, LUA_REGISTRYINDEX);  
}

void lua_registry_erase(lua_state* L, void* object){
    lua_pushlightuserdata(L, object);  /* push address as key */
    lua_pushnil(L);  		       /* push nil as value */
    lua_settable(L, LUA_REGISTRYINDEX);  
}

class LuaModule {
public:
	LuaModule(lua_state* L) : L(L){
		lua_register_object(L, this);
	}
	~LuaModule(){
		lua_deregister_object(L, this);
	}
	void set_function(const char* key, lua_CFunction value);
	void set_number(const char* key, double value);
	void set_newtable(const char* key);
	void set_yaml(const char* key, const YAML::Node* root);
	void pop(const char* key);
	void push(const char* key);
private:
	lua_state* L;
};



	
void LuaModule::pop(const char* key){
	lua_registry_push(L, this); /*Get the associated lua table*/	
	int ind = lua_gettop(L);

	lua_pushstring(L, key);	
	lua_pushcfunction(L, value); /**/
	lua_settable(L, value);
	lua_pop(L, 1);
}
	
void LuaModule::set_function(const char* key, lua_CFunction value){
	lua_registry_push(L, this); /*Get the associated lua table*/	
	int ind = lua_gettop(L);
	lua_pushstring(L, key);	
	lua_pushcfunction(L, value); /**/
	lua_settable(L, value);
	lua_pop(L, 1);
}
void LuaModule::set_number(const char* key, double value){
	lua_pushstring(L, key);	
	lua_pushnumber(L, value); /**/
	lua_registry_push(L, this); /*Get the associated lua table*/	
}
#endif
