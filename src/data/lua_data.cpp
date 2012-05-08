extern "C" {
	#include <lua/lua.h>
	#include <lua/lauxlib.h>
	#include <lua/lualib.h>
}

#include "lua_data.h"
#include "../world/lua/lua_api.h"

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


void luadata_step_event(lua_State* lua_state, const LuaData& ld, obj_id id){
	luadata_push(lua_state, ld);
	int tableind = lua_gettop(lua_state);
	lua_pushstring(lua_state, "step");
	lua_gettable(lua_state, tableind);
	lua_pushgameinst(lua_state, id);
	lua_call(lua_state, 1, 0);
}
void luadata_draw_event(lua_State* lua_state, const LuaData& ld, obj_id id){
	luadata_push(lua_state, ld);
	int tableind = lua_gettop(lua_state);
	lua_pushstring(lua_state, "draw");
	lua_gettable(lua_state, tableind);
	lua_pushgameinst(lua_state, id);
	lua_call(lua_state, 1, 0);
}
