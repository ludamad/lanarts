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

//lua_id lua_alloc_id(lua_State* L){
//	lua_rawgeti(L, LUA_REGISTRYINDEX, 0);
//	double idx = lua_tonumber(L, lua_gettop(L));
//	lua_pop(L, 1);
//	lua_pushnumber(L, idx+1);
//	lua_rawseti(L, LUA_REGISTRYINDEX, 0);
//
//	return idx;
//}
//
//void lua_free_id(lua_State* L){
//	lua_table_remove(L, LUA_REGISTRYINDEX);
//}
//
//void lua_push_by_id(lua_State* L, lua_id id){
//	lua_rawgeti(L, LUA_REGISTRYINDEX, id);
//}
//void lua_set_by_id(lua_State* L, lua_id id){
//	lua_rawseti(L, LUA_REGISTRYINDEX, id);
//}
//

