/*
 * lua_object_funcs.cpp:
 *  Functions callable from lua that control creation of various
 *  GameInst objects.
 */

#include <lua.hpp>

#include "data/lua_game_data.h"

#include "draw/colour_constants.h"

#include "gamestate/GameState.h"

#include "objects/player/PlayerInst.h"

#include "objects/GameInst.h"
#include "util/math_util.h"

#include "lua_api.h"

static int players_in_level(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	std::vector<PlayerInst*> players = gs->players_in_level();
	lua_createtable(L, 0, 0);
	int table = lua_gettop(L);
	for (int i = 0; i < players.size(); i++) {
		lua_push_gameinst(L, players[i]);
		//  lua_pushnumber(L, 2);
		lua_rawseti(L, table, i + 1);
	}
	return 1;
}
// Returns monsters in level
static int monsters_in_level(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	const std::vector<obj_id>& monsters =
			gs->monster_controller().monster_ids();
	lua_newtable(L);
	int tableidx = lua_gettop(L);

	int valid = 1;
	for (int i = 0; i < monsters.size(); i++) {
		GameInst* e = gs->get_instance(monsters[i]);
		if (e) {
			lua_push_gameinst(L, e);
			lua_rawseti(L, tableidx, valid++);
		}
	}

	return 1;
}
// Returns monsters seen
// Take arguments: optional player, otherwise all players seen by; returns objects
static int monsters_seen(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int narg = lua_gettop(L);
	PlayerInst* p = narg >= 1 ? (PlayerInst*)lua_gameinst_arg(L, 1) : NULL;
	const std::vector<obj_id>& monsters =
			gs->monster_controller().monster_ids();
	lua_newtable(L);
	int tableidx = lua_gettop(L);

	int valid = 1;
	for (int i = 0; i < monsters.size(); i++) {
		GameInst* e = gs->get_instance(monsters[i]);
		if (e && gs->object_visible_test(e, p, false)) {
			lua_push_gameinst(L, e);
			lua_rawseti(L, tableidx, valid++);
		}
	}

	return 1;
}

static int reveal_map(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	gs->tiles().mark_all_seen();
	return 0;
}

static int obj_to_exit(lua_State* L) {

	GameState* gs = lua_api::gamestate(L);
	GameLevelState* level = gs->get_level();
	GameInst* user = lua_gameinst_arg(L, 1);
	MTwist& mt = gs->rng();
	int nexits = level->exits.size();
	if (nexits == 0) {
		gs->game_chat().add_message("You feel disoriented! You do not move.",
				COL_PALE_RED);
		return 0;
	}
	int exit = mt.rand(nexits);
	Pos p = level->exits[exit].entrancesqr;
	p = centered_multiple(p, TILE_SIZE);
	user->update_position(p.x, p.y);
	gs->game_chat().add_message("You are yanked to the exit!", COL_PALE_GREEN);

	PlayerInst* player = dynamic_cast<PlayerInst*>(user);
	gs->view().sharp_center_on(player->pos());

	return 0;
}

void lua_object_func_bindings(lua_State* L) {
	//Use C function name as lua function name:
#define LUA_FUNC_REGISTER(f) \
	lua_pushcfunction(L, f); \
	lua_setfield(L, LUA_GLOBALSINDEX, #f);

	LUA_FUNC_REGISTER(monsters_in_level);
	LUA_FUNC_REGISTER(players_in_level);
	LUA_FUNC_REGISTER(monsters_seen);
	LUA_FUNC_REGISTER(reveal_map);
	LUA_FUNC_REGISTER(obj_to_exit);
}
