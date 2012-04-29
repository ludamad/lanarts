/*
 * lua_data.h
 *
 *  Created on: Apr 28, 2012
 *      Author: 100397561
 */

#ifndef LUA_DATA_H_
#define LUA_DATA_H_
#include <string>

struct LuaData {
	int table_id;
	std::string table_index;
	LuaData(int table_id, const std::string& str) : table_id(table_id), table_index(str){
	}
	LuaData(const std::string& str);
};

void luadata_call(lua_State* lua_state, const LuaData& ld);
void luadata_push(lua_State* lua_state, const LuaData& ld);
void luadata_pop(lua_State* lua_state, const LuaData& ld);

#endif /* LUA_DATA_H_ */
