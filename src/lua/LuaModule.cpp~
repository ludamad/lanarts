#if 0
extern "C" {
	#include <lua/lua.h>
}

void lua_register_object(lua_state* L, void* object){
    lua_pushlightuserdata(L, object);  /* push address as key */
    lua_pushlightuserdata(L, object);  /* push address as value */
    lua_settable(L, LUA_REGISTRYINDEX);  
}

void lua_deregister_object(lua_state* L, void* object){
    lua_pushlightuserdata(L, object);  /* push address as key */
    lua_pushnil(L);  		       /* push nil as value */
    lua_settable(L, LUA_REGISTRYINDEX);  
}

class LuaModule {
public:
	LuaModule(GameState* gs){
		L = gs->get_lua_state();
		lua_register_object(L, this);
	}
	~LuaModule(){
		lua_deregister_object(L, this);
	}
	
private:
	lua_state* L;
};
#endif
