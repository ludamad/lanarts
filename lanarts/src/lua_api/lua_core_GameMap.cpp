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

static int gmap_regenerate(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int levelid = gs->get_level()->id();

	if (lua_gettop(L) > 0) {
		levelid = luaL_checknumber(L, 1);
	}

	gs->game_world().regen_level(levelid);
	return 0;
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

static int gmap_object_collisions(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	GameInst* inst = luawrap::get<GameInst*>(L, 1);
	GameInst* objects[MAX_RET];

	int nret = gs->object_radius_test(inst, objects, MAX_RET);
	lua_newtable(L);
	for (int i = 0; i < nret; i++){
		luawrap::push(L, objects[i]);
		lua_rawset(L, i+1);
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

namespace lua_api {
	void register_lua_core_maps(lua_State* L) {
		LuaValue gmap = register_lua_submodule(L, "core.GameMap");
		gmap["create"].bind_function(gmap_create);
		gmap["transfer"].bind_function(gmap_transfer);
		gmap["TILE_SIZE"] = TILE_SIZE;

		// Query functions:
		gmap["objects_list"].bind_function(gmap_objects_list);
		gmap["objects"].bind_function(gmap_objects);

		gmap["add_instance"].bind_function(gmap_add_instance);
		gmap["instance"].bind_function(gmap_instance);

		gmap["monsters_list"].bind_function(gmap_monsters_list);
		gmap["monsters"].bind_function(gmap_monsters);

		gmap["object_visible"].bind_function(gmap_object_visible);
		gmap["object_collisions"].bind_function(gmap_object_collisions);
		gmap["object_place_free"].bind_function(gmap_object_place_free);
		gmap["place_free"].bind_function(gmap_place_free);
		gmap["radius_place_free"].bind_function(gmap_radius_place_free);

		// Debug/special-case-only functions:
		gmap["regenerate"].bind_function(gmap_regenerate);
	}
}
