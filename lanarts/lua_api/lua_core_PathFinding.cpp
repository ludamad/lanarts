/*
 * lua_core_PathFinding.cpp:
 *  Pathfinding submodule
 */

#include <SDL.h>

#include <ldungeon_gen/Map.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>

#include "lua_newapi.h"

#include "lua_api/lua_newapi.h"
#include "pathfind/AStarPath.h"

#include "gamestate/GameState.h"
#include "gamestate/GameMapState.h"

typedef smartptr<AStarPath> AStarPathPtr;
static int astar_calculate_path(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	AStarPathPtr astar = luawrap::get<AStarPathPtr>(L, 1);
	level_id gmap = luawrap::get<int>(L, 2);
	Pos from_xy = luawrap::get<Pos>(L, 3);
	Pos to_xy = luawrap::get<Pos>(L, 4);
	bool clear = lua_gettop(L) >= 5 ? lua_toboolean(L, 5) : true;

	GameMapState* previous_level = gs->get_level();
	gs->set_level(gs->game_world().get_level(gmap));
	luawrap::push(L, astar->calculate_AStar_path(gs, from_xy, to_xy, clear));
	gs->set_level(previous_level);

	return 1;
}

LuaValue lua_astarmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "AStarBuffer");
	LuaValue methods = luameta_constants(meta);
	methods["calculate_path"].bind_function(astar_calculate_path);

	luameta_gc<AStarPathPtr>(meta);

	return meta;
}

static AStarPathPtr astar_buffer_create() {
	return AStarPathPtr(new AStarPath());
}

namespace lua_api {
	void register_lua_core_PathFinding(lua_State* L) {
		LuaValue paths = register_lua_submodule(L, "core.PathFinding");
		luawrap::install_userdata_type<AStarPathPtr, &lua_astarmetatable>();
		paths["astar_buffer_create"].bind_function(astar_buffer_create);
	}
}
