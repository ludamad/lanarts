/*
 * lua_levelgen_funcs.cpp:
 *  Lua level gen callbacks
 */

#include "lua_levelgen_funcs.h"

static GeneratedLevel* lua_togenlevel(lua_State* L, int idx) {
	return *((GeneratedLevel**) lua_touserdata(L, idx));
}

static Range lua_torange(lua_State* L) {
	Range r;
}

static int lua_roomgen(lua_State* L) {
	GeneratedLevel* level = lua_togenlevel(L, 1);
	int room_padding = lua_tonumber(L, 2);
	Range size;
	return 1;
}

void lua_generatedlevel_push(lua_State* L, GeneratedLevel& level) {
	void* data = lua_newuserdata(L, sizeof(GeneratedLevel*));
	*((GeneratedLevel**) data) = &level;
}

void lua_generatedlevel_invalidate(lua_State* L, int idx) {
	void* data = lua_touserdata(L, idx);
	*((GeneratedLevel**) data) = NULL;
}

void lua_levelgen_func_bindings(lua_State* L) {

}
