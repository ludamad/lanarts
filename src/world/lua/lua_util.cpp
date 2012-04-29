/*
 * lua_util.cpp
 *
 *  Created on: Apr 28, 2012
 *      Author: 100397561
 */

#include "lua_util.h"


extern "C" {
    #include <lua/lua.h>
    #include <lua/lauxlib.h>
    #include <lua/lualib.h>
}

//lua_id lua_alloc_id(lua_State* lua_state){
//	lua_rawgeti(lua_state, LUA_REGISTRYINDEX, 0);
//	double idx = lua_tonumber(lua_state, lua_gettop(lua_state));
//	lua_pop(lua_state, 1);
//	lua_pushnumber(lua_state, idx+1);
//	lua_rawseti(lua_state, LUA_REGISTRYINDEX, 0);
//
//	return idx;
//}
//
//void lua_free_id(lua_State* lua_state){
//	lua_table_remove(lua_state, LUA_REGISTRYINDEX);
//}
//
//void lua_push_by_id(lua_State* lua_state, lua_id id){
//	lua_rawgeti(lua_state, LUA_REGISTRYINDEX, id);
//}
//void lua_set_by_id(lua_State* lua_state, lua_id id){
//	lua_rawseti(lua_state, LUA_REGISTRYINDEX, id);
//}
//

