/*
 * lua_core_maps.cpp:
 *  Implements a lua submodule for lanarts.
 */

#include <SDL.h>

#include <ldungeon_gen/Map.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/types.h>

#include "gamestate/GameState.h"
#include "objects/enemy/EnemyInst.h"
#include "objects/player/PlayerInst.h"
#include "gamestate/GameMapState.h"
#include "gamestate/GameSettings.h"

#include "draw/TileEntry.h"

#include "stats/ClassEntry.h"

#include "lua_newapi.h"

#include "lua_api/lua_api.h"
#include "lua_api/lua_gameinst.h"

static int maps_create(LuaStackValue args) {
	using namespace luawrap;
	using namespace ldungeon_gen;

	GameState* gs = lua_api::gamestate(args);
	MapPtr map = args["map"].as<MapPtr>();
	GameMapState* game_map = gs->game_world().map_create(map->size(),
			defaulted(args["wandering_enabled"], true));

	game_map->label() = defaulted(args["label"], std::string("Somewhere"));

	GameTiles& tiles = game_map->tiles();
	BBox bbox(Pos(), tiles.size());
	FOR_EACH_BBOX(bbox, x, y) {
		Tile& tile = tiles.get(Pos(x,y));
		Square& square = (*map)[Pos(x,y)];
		TileEntry& entry = res::tile(square.content);

		int variations = entry.images.size();
		tile.tile = square.content;
		tile.subtile = gs->rng().rand(variations);
		(*tiles.solidity_map())[Pos(x,y)] = ((square.flags & FLAG_SOLID) != 0);
		(*tiles.seethrough_map())[Pos(x,y)] = ((square.flags & FLAG_SEETHROUGH) != 0);
	}

	if (!args["instances"].isnil()) {
		typedef std::vector<GameInst*> InstanceList;
		InstanceList instances = args["instances"].as<InstanceList>();
		for (int i = 0; i < instances.size(); i++) {
			gs->add_instance(game_map->id(), instances[i]);
		}
	}

	return game_map->id();
}

static void maps_transfer(LuaStackValue inst, level_id map, Pos xy) {
	GameState* gs = lua_api::gamestate(inst);
	level_id current_id = gs->game_world().get_current_level_id();
	gs->game_world().level_move(inst.as<GameInst*>()->id, xy.x, xy.y, current_id, map);
}

namespace lua_api {
	void register_lua_core_maps(lua_State* L) {
		LuaValue submodule = register_lua_submodule(L, "core.maps");
		submodule["create"].bind_function(maps_create);
		submodule["transfer"].bind_function(maps_transfer);
		submodule["TILE_SIZE"] = TILE_SIZE;
	}
}
