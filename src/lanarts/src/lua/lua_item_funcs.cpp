/*
 * lua_item_funcs.cpp:
 *  Functions callable from lua that query various item properties
 */

#include <lua.hpp>

#include "data/lua_game_data.h"

#include "gamestate/GameState.h"

#include "objects/GameInst.h"

#include "objects/ScriptedInst.h"
#include "lua_api.h"
//
//// Returns monsters seen
//// Take arguments: optional player, otherwise all players seen by; returns objects
//static int item_is_weapon(lua_State* L) {
//	GameState* gs = lua_get_gamestate(L);
//	int narg = lua_gettop(L);
//	PlayerInst* p = narg >= 1 ? (PlayerInst*)lua_gameinst_arg(L, 1) : NULL;
//	const std::vector<obj_id>& monsters =
//			gs->monster_controller().monster_ids();
//	lua_newtable(L);
//	int tableidx = lua_gettop(L);
//
//	int valid = 1;
//	for (int i = 0; i < monsters.size(); i++) {
//		GameInst* e = gs->get_instance(monsters[i]);
//		if (e && gs->object_visible_test(e, p, false)) {
//			lua_push_gameinst(L, e);
//			lua_rawseti(L, tableidx, valid++);
//		}
//	}
//
//	return 1;
//}
//
//void lua_object_func_bindings(lua_State* L) {
//	//Use C function name as lua function name:
//#define LUA_FUNC_REGISTER(f) \
//	lua_pushcfunction(L, f); \
//	lua_setfield(L, LUA_GLOBALSINDEX, #f);
//
//	LUA_FUNC_REGISTER(obj_create);
//	LUA_FUNC_REGISTER(monsters_in_level);
//	LUA_FUNC_REGISTER(monsters_seen);
//}
