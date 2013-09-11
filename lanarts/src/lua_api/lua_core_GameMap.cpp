/*
 * lua_core_GameMap.cpp:
 *	core.GameMap submodule: Handles level query functions.
 *	TODO: Phase out implicit 'this level'
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

static int gmap_create(LuaStackValue args) {
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

// level functions

static GameInst* gmap_lookup(LuaStackValue current_map, obj_id object) {
	GameState* gs = lua_api::gamestate(current_map);
	GameMapState* map = gs->game_world().get_level(current_map.to_int());
	return map->game_inst_set().get_instance(object);
}

static int gmap_objects_list(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	GameInstSet& insts = gs->get_level()->game_inst_set();
	luawrap::push(L, insts.to_vector());
	return 1;
}

static int gmap_objects(lua_State* L) {
	lua_pushcfunction(L, lua_api::l_itervalues);
	gmap_objects_list(L);
	lua_call(L, 1, 1);
	return 1;
}

static bool gmap_tile_is_solid(LuaStackValue current_map, Pos xy) {
	GameState* gs = lua_api::gamestate(current_map);
	GameMapState* map = gs->game_world().get_level(current_map.to_int());
	return map->tiles().is_solid(xy);
}

static bool gmap_tile_is_seethrough(LuaStackValue current_map, Pos xy) {
	GameState* gs = lua_api::gamestate(current_map);
	GameMapState* map = gs->game_world().get_level(current_map.to_int());
	return map->tiles().is_seethrough(xy);
}

static void gmap_tile_set_solid(LuaStackValue current_map, Pos xy, bool solidity) {
	GameState* gs = lua_api::gamestate(current_map);
	GameMapState* map = gs->game_world().get_level(current_map.to_int());
	map->tiles().set_solid(xy, solidity);
}

static void gmap_tile_set_seethrough(LuaStackValue current_map, Pos xy, bool solidity) {
	GameState* gs = lua_api::gamestate(current_map);
	GameMapState* map = gs->game_world().get_level(current_map.to_int());
	map->tiles().set_seethrough(xy, solidity);
}

static bool gmap_tile_was_seen(LuaStackValue current_map, Pos xy) {
	GameState* gs = lua_api::gamestate(current_map);
	GameMapState* map = gs->game_world().get_level(current_map.to_int());
	return map->tiles().was_seen(xy);
}

static void gmap_add_instance(LuaStackValue gameinst) {
	GameInst* inst = gameinst.as<GameInst*>();
	if (inst->current_floor != -1) {
		luawrap::error("Attempt to add game instance that was already added!");
	}
	lua_api::gamestate(gameinst)->add_instance(inst);
}

static int gmap_monsters_list(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	const std::vector<obj_id>& mons = gs->monster_controller().monster_ids();
	lua_newtable(L);
	for (int i = 0; i < mons.size(); i++) {
		luawrap::push(L, gs->get_instance(mons[i]));
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

static int gmap_monsters_seen(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int narg = lua_gettop(L);
	PlayerInst* p = narg >= 1 ? luawrap::get<PlayerInst*>(L, 1) : NULL;
	const std::vector<obj_id>& monsters =
			gs->monster_controller().monster_ids();
	lua_newtable(L);
	int tableidx = lua_gettop(L);

	int valid = 1;
	for (int i = 0; i < monsters.size(); i++) {
		GameInst* e = gs->get_instance(monsters[i]);
		if (e && gs->object_visible_test(e, p, false)) {
			luawrap::push(L, e);
			lua_rawseti(L, tableidx, valid++);
		}
	}

	return 1;
}

static int gmap_monsters(lua_State* L) {
	lua_pushcfunction(L, lua_api::l_itervalues);
	gmap_monsters_list(L);
	lua_call(L, 1, 1);
	return 1;
}

static int gmap_distance_to_player(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int map_id = lua_tointeger(L, 1);
	Pos xy = luawrap::get<Pos>(L, 2);
	std::vector<PlayerDataEntry>& entries = gs->player_data().all_players();
	int max_dist = -1;
	for (int i = 0; i < entries.size(); i++) {
		GameInst* player = entries[i].player_inst.get();
		Pos dpos = xy - player->pos();
		max_dist = std::max(max_dist, abs(dpos.x));
		max_dist = std::max(max_dist, abs(dpos.y));
	}

	if (max_dist == -1) {
		lua_pushnil(L);
	} else {
		lua_pushinteger(L, max_dist);
	}
	return 1;
}

// Look up a specific instance given an id
static int gmap_instance(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	luawrap::push(L, gs->get_instance(luaL_checkinteger(L, 1)));
	return 1;
}

// TODO add test for specific player
static bool gmap_object_visible(const LuaStackValue& obj) {
	GameState* gs = lua_api::gamestate(obj);
	return gs->object_visible_test(obj.as<GameInst*>());
}

static int gmap_object_place_free(lua_State* L) {
	int nargs = lua_gettop(L);
	GameState* gs = lua_api::gamestate(L);
	GameInst* inst = luawrap::get<GameInst*>(L, 1);
	Pos p = inst->pos();

	if (nargs > 1) {
		p = luawrap::get<Pos>(L, 2);
	}

	lua_pushboolean(L, !gs->solid_test(inst, p.x, p.y));
	return 1;
}

const int MAX_RET = 16;

static int gmap_object_collision_check(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	GameInst* inst = luawrap::get<GameInst*>(L, 1);
	Pos pos = inst->pos();
	if (lua_gettop(L) >= 2) {
		pos = luawrap::get<Pos>(L, 2);
	}
	GameInst* objects[MAX_RET];

	int nret = gs->object_radius_test(inst, objects, MAX_RET, NULL, pos.x, pos.y);
	lua_newtable(L);
	for (int i = 0; i < nret; i++){
		luawrap::push(L, objects[i]);
		lua_rawseti(L, -2, i+1);
	}
	return 1;
}

static std::vector<GameInst*> gmap_rectangle_collision_check(LuaStackValue current_map, BBox area, LuaStackValue tester /* Can be empty */) {
	GameState* gs = lua_api::gamestate(current_map);
	GameMapState* map = gs->game_world().get_level(current_map.to_int());

	return map->game_inst_set().object_rectangle_test(area, tester.isnil() ? NULL : tester.as<GameInst*>());
}

static int gmap_object_tile_check(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	GameInst* inst = luawrap::get<GameInst*>(L, 1);
	Pos pos = inst->pos();
	if (lua_gettop(L) >= 2) {
		pos = luawrap::get<Pos>(L, 2);
	}

	Pos hit_pos;
	bool collided = gs->tile_radius_test(pos.x, pos.y, inst->radius, true, -1, &hit_pos);
	if (!collided) {
		lua_pushnil(L);
	} else {
		luawrap::push(L, hit_pos);
	}
	return 1;
}

static bool gmap_place_free(const LuaStackValue& pos) {
	GameState* gs = lua_api::gamestate(pos);
	Pos p = pos.as<Pos>();
	return !gs->solid_test(NULL, p.x, p.y, 1);
}

static bool gmap_radius_place_free(const LuaStackValue& radius,
		const LuaStackValue& pos) {
	GameState* gs = lua_api::gamestate(radius);
	Pos p = pos.as<Pos>();
	return !gs->solid_test(NULL, p.x, p.y, radius.as<int>());
}


static void gmap_transfer(LuaStackValue inst, level_id map, Pos xy) {
	GameState* gs = lua_api::gamestate(inst);
	level_id current_id = gs->game_world().get_current_level_id();
	gs->game_world().level_move(inst.as<GameInst*>()->id, xy.x, xy.y, current_id, map);
}

static void gmap_map_step(LuaStackValue table) {
	GameState* gs = lua_api::gamestate(table);
	level_id map_id = table["map"].to_int();
	gs->game_world().get_level(map_id)->step(gs);
}

static void gmap_map_draw(LuaStackValue table) {
	GameState* gs = lua_api::gamestate(table);
	level_id map_id = table["map"].to_int();
	gs->game_world().get_level(map_id)->draw(gs);
}

namespace lua_api {
	void register_lua_core_maps(lua_State* L) {
		LuaValue gmap = register_lua_submodule(L, "core.GameMap");
		gmap["create"].bind_function(gmap_create);
		gmap["transfer"].bind_function(gmap_transfer);
		gmap["TILE_SIZE"] = TILE_SIZE;

		// Query functions:
		gmap["lookup"].bind_function(gmap_lookup);

		gmap["objects_list"].bind_function(gmap_objects_list);
		gmap["objects"].bind_function(gmap_objects);

		gmap["tile_is_solid"].bind_function(gmap_tile_is_solid);
		gmap["tile_is_seethrough"].bind_function(gmap_tile_is_seethrough);
		gmap["tile_set_solid"].bind_function(gmap_tile_set_solid);
		gmap["tile_set_seethrough"].bind_function(gmap_tile_set_seethrough);
		gmap["tile_was_seen"].bind_function(gmap_tile_was_seen);

		gmap["add_instance"].bind_function(gmap_add_instance);
		gmap["instance"].bind_function(gmap_instance);

		gmap["monsters_list"].bind_function(gmap_monsters_list);
		gmap["monsters"].bind_function(gmap_monsters);

		gmap["map_step"].bind_function(gmap_map_step);
		gmap["map_draw"].bind_function(gmap_map_draw);

		gmap["distance_to_player"].bind_function(gmap_distance_to_player);

		gmap["object_visible"].bind_function(gmap_object_visible);
		gmap["object_collision_check"].bind_function(gmap_object_collision_check);
		gmap["object_tile_check"].bind_function(gmap_object_tile_check);
		gmap["object_place_free"].bind_function(gmap_object_place_free);
		gmap["rectangle_collision_check"].bind_function(gmap_rectangle_collision_check);

		gmap["place_free"].bind_function(gmap_place_free);
		gmap["radius_place_free"].bind_function(gmap_radius_place_free);
	}
}