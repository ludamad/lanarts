/*
 * lua_object_funcs.cpp:
 *  Functions callable from lua that control creation of various
 *  GameInst objects.
 */

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

#include "lua_api.h"

#include "../data/lua_game_data.h"

#include "../world/GameState.h"

#include "../world/objects/GameInst.h"
#include "../world/objects/ScriptedInst.h"

// Creates object, adding to game world, returns said object
// Take arguments: objtype, x, y; returns obj
static int obj_create(lua_State* L) {
	scriptobj_id scr_obj = scriptobject_from_lua(L, 1);
	int x = lua_tonumber(L, 2), y = lua_tonumber(L, 3);

	ScriptedInst* inst = new ScriptedInst(scr_obj, x, y);

	// Add to world and return
	GameState* gs = lua_get_gamestate(L);
	gs->add_instance(inst);
	lua_push_gameinst(L, inst);

	return 1;
}

void lua_object_func_bindings(lua_State* L) {
	//Use C function name as lua function name:
#define LUA_FUNC_REGISTER(f) \
	lua_pushcfunction(L, f); \
	lua_setfield(L, LUA_GLOBALSINDEX, #f);

	LUA_FUNC_REGISTER(obj_create);
}
