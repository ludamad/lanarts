extern "C" {
	#include <lua/lua.h>
	#include <lua/lauxlib.h>
	#include <lua/lualib.h>
}

#include "lua_data.h"

LuaData::LuaData(const std::string& str) : table_id(LUA_GLOBALSINDEX), table_index(str) {

}


void luadata_call(lua_State* lua_state, const LuaData& ld){
	luadata_push(lua_state, ld);
	lua_call(lua_state, 0, 0);
}
void luadata_push(lua_State* lua_state, const LuaData& ld){
	lua_getfield(lua_state, ld.table_id, ld.table_index.c_str());
}
void luadata_pop(lua_State* lua_state, const LuaData& ld){
	lua_setfield(lua_state, ld.table_id, ld.table_index.c_str());
}
