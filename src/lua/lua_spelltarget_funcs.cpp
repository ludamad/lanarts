/*
 * lua_object_funcs.cpp:
 *  Functions for spell auto-targetting
 */

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

#include "lua_api.h"

#include "../data/lua_game_data.h"

#include "../util/math_util.h"

#include "../gamestate/GameState.h"

#include "../objects/GameInst.h"
#include "../objects/player/PlayerInst.h"

// Take arguments: caster, target; returns x, y
static int spell_choose_safest_square(lua_State* L) {
	GameState* gs = lua_get_gamestate(L);
	PlayerInst* inst = dynamic_cast<PlayerInst*>(lua_gameinst_arg(L, 1));
	//CombatGameInst* target = dynamic_cast<CombatGameInst*>(lua_gameinst_arg(L, 2));
	Pos p;
	if (find_safest_square(inst, gs, p)) {
		lua_pushnumber(L, round_to_multiple(p.x, TILE_SIZE, true));
		lua_pushnumber(L, round_to_multiple(p.y, TILE_SIZE, true));
		return 2;
	} else {
		lua_pushnil(L);
		return 1;
	}
}

// Take arguments: obj, x, y; returns x, y
static int spell_choose_target(lua_State* L) {
	GameState* gs = lua_get_gamestate(L);
	//PlayerInst* inst = dynamic_cast<PlayerInst*>(lua_gameinst_arg(L, 1));
	CombatGameInst* target = dynamic_cast<CombatGameInst*>(lua_gameinst_arg(L,
			2));
	if (target) {
		lua_pushnumber(L, target->x);
		lua_pushnumber(L, target->y);
		return 2;
	} else {
		lua_pushnil(L);
		return 1;
	}
}

void lua_spelltarget_bindings(lua_State* L) {
	//Use C function name as lua function name:
#define LUA_FUNC_REGISTER(f) \
	lua_pushcfunction(L, f); \
	lua_setfield(L, LUA_GLOBALSINDEX, #f);

	LUA_FUNC_REGISTER(spell_choose_safest_square);
	LUA_FUNC_REGISTER(spell_choose_target);
}
