extern "C" {
	#include <lua/lua.h>
	#include <lua/lauxlib.h>
	#include <lua/lualib.h>
}

#include "lua_data.h"
#include "../world/lua/lua_api.h"

LuaData::LuaData(const std::string& str) : table_id(LUA_GLOBALSINDEX), table_index(str) {

}


void luadata_call(lua_State* L, const LuaData& ld){
	luadata_push(L, ld);
	lua_call(L, 0, 0);
}
void luadata_push(lua_State* L, const LuaData& ld){
	lua_getfield(L, ld.table_id, ld.table_index.c_str());
}
void luadata_pop(lua_State* L, const LuaData& ld){
	lua_setfield(L, ld.table_id, ld.table_index.c_str());
}


void luadata_step_event(lua_State* L, const LuaData& ld, obj_id id){
	luadata_push(L, ld);
	int tableind = lua_gettop(L);
	lua_pushstring(L, "step");
	lua_gettable(L, tableind);
	lua_pushgameinst(L, id);
	lua_call(L, 1, 0);
}
void luadata_draw_event(lua_State* L, const LuaData& ld, obj_id id){
	luadata_push(L, ld);
	int tableind = lua_gettop(L);
	lua_pushstring(L, "draw");
	lua_gettable(L, tableind);
	lua_pushgameinst(L, id);
	lua_call(L, 1, 0);
}
