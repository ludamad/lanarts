/*
 * lua_core_GameMap.cpp:
 *    core.GameMap submodule: Handles level query functions.
 *    TODO: Phase out implicit 'this level'
 */

#include <SDL.h>

#include <ldungeon_gen/Map.h>

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/luameta.h>

#include "gamestate/GameState.h"
#include "objects/EnemyInst.h"
#include "objects/PlayerInst.h"
#include "gamestate/GameMapState.h"
#include "gamestate/GameSettings.h"
#include "gamestate/TeamIter.h"

#include "draw/TileEntry.h"

#include "stats/ClassEntry.h"

#include "lua_api.h"

#include "lua_api/lua_api.h"

template <typename T>
static T stack_defaulted(lua_State* L, int narg, const T& default_value) {
    return lua_gettop(L) >= narg ? luawrap::get<T>(L, narg) : default_value;
}

static level_id mapid(LuaField map_obj) {
    return map_obj["_id"].to_int();
}

GameMapState* mapstate(LuaStackValue map_obj, bool defaulted = false) {
    GameState* gs = lua_api::gamestate(map_obj);
    level_id id;
    if (defaulted && map_obj.isnil()) {
        id = gs->get_level_id();
    } else {
        id = map_obj["_id"].to_int();
    }
    return gs->game_world().get_level(id);
}

// TODO: This is a stopgap measure until we achieve true separation of maps.
#define LEVEL_WRAPPED(method_body) \
    /* Become current level */ \
    GameMapState* previous_level = gs->get_level(); \
    gs->set_level(new_level); \
    method_body ; \
    /* Return to previous level */ \
    gs->set_level(previous_level)

static int gmap_tiles_predraw(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    GameMapState* new_level = mapstate(LuaStackValue(L, 1));
    bool reveal_all = stack_defaulted(L, 2, true);
    LEVEL_WRAPPED(
        gs->tiles().pre_draw(gs, reveal_all);
    );
    return 0;
}

static int gmap_tiles_postdraw(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    GameMapState* new_level = mapstate(LuaStackValue(L, 1));
    LEVEL_WRAPPED(
        gs->tiles().post_draw(gs);
    );
    return 0;
}

static int gmap_instances_draw(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    GameMapState* new_level = mapstate(LuaStackValue(L, 1));
    LEVEL_WRAPPED(
        std::vector<GameInst*> safe_copy = new_level->game_inst_set().to_vector();
    );
    for (size_t i = 0; i < safe_copy.size(); i++) {
        safe_copy[i]->draw(gs);
    }
    return 0;
}

static void gmap_destroy(LuaStackValue map_obj) {
    GameState* gs = lua_api::gamestate(map_obj);
    lua_pushnil(map_obj.luastate());
    gs->game_world().pop_level_object(mapid(map_obj));
}

static void gmap_init(LuaStackValue map_obj, LuaStackValue args) {
    using namespace luawrap;
    using namespace ldungeon_gen;

    GameState* gs = lua_api::gamestate(map_obj);
    MapPtr map = args["map"].as<MapPtr>();
    GameMapState* game_map = gs->game_world().map_create(map->size(), map,
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

    map_obj["_id"] = game_map->id();
    map_obj.push();
    gs->game_world().pop_level_object(game_map->id());
}


static int gmap_create(lua_State* L) {
    lua_newtable(L);
    gmap_init(LuaStackValue(L, -1), LuaStackValue(L, 1));
    return 1;
}

LuaValue lua_mapmetatable(lua_State* L) {
    LuaValue meta = luameta_new(L, "Map");
    LuaValue methods = luameta_constants(meta);

    methods["init"].bind_function(gmap_init);
    methods["destroy"].bind_function(gmap_destroy);

    return meta;
}

static int gmap_maptype(lua_State* L) {
    lua_newtable(L);
    luameta_push(L, lua_mapmetatable);
    lua_setfield(L, -2, "parent");
    luawrap::globals(L)["newtype"].push();
    lua_call(L, 1, 1);
    return 1;
}

// level functions

static GameInst* gmap_lookup(LuaStackValue map_obj, obj_id object) {
    GameState* gs = lua_api::gamestate(map_obj);
    GameMapState* map = mapstate(map_obj);
    return map->game_inst_set().get_instance(object);
}

static int gmap_objects_list(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    level_id id = -1;
    if (lua_gettop(L) == 0) {
        id = gs->get_level_id();
    } else {
        id = LuaStackValue(L, 1)["_id"].to_int();
    }
    auto* level = gs->game_world().get_level(id);
    GameInstSet& insts = level->game_inst_set();
    luawrap::push(L, insts.to_vector());
    return 1;
}

static int gmap_map_label(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    level_id id = -1;
    if (lua_gettop(L) == 0) {
        id = gs->get_level_id();
    } else {
        id = LuaStackValue(L, 1)["_id"].to_int();
    }
    auto* level = gs->game_world().get_level(id);
    luawrap::push(L, level->label());
    return 1;
}

static bool gmap_tile_is_solid(LuaStackValue map_obj, Pos xy) {
    GameState* gs = lua_api::gamestate(map_obj);
    GameMapState* map = mapstate(map_obj);
    return map->tiles().is_solid(xy);
}

static bool gmap_tile_is_seethrough(LuaStackValue map_obj, Pos xy) {
    GameState* gs = lua_api::gamestate(map_obj);
    GameMapState* map = mapstate(map_obj);
    return map->tiles().is_seethrough(xy);
}

static void gmap_tile_set_solid(LuaStackValue map_obj, Pos xy, bool solidity) {
    GameState* gs = lua_api::gamestate(map_obj);
    GameMapState* map = mapstate(map_obj);
    map->tiles().set_solid(xy, solidity);
}

static void gmap_tile_set_seethrough(LuaStackValue map_obj, Pos xy, bool solidity) {
    GameState* gs = lua_api::gamestate(map_obj);
    GameMapState* map = mapstate(map_obj);
    map->tiles().set_seethrough(xy, solidity);
}

static bool gmap_tile_was_seen(LuaStackValue map_obj, Pos xy) {
    GameState* gs = lua_api::gamestate(map_obj);
    GameMapState* map = mapstate(map_obj);
    return map->tiles().was_seen(xy);
}

static void gmap_add_object(LuaStackValue map_obj, LuaStackValue gameinst) {
    GameState* gs = lua_api::gamestate(map_obj);
    GameInst* inst = gameinst.as<GameInst*>();
    if (inst->current_floor != -1) {
        luawrap::error("Attempt to add game instance that was already added!");
    }
    gs->add_instance(map_obj["_id"].to_int(), inst);
}

static int gmap_players_list(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    luawrap::push(L, gs->players_in_level());
    return 1;
}

static int gmap_monsters_list(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    const std::vector<obj_id>& mons = gs->monster_controller().monster_ids();
    lua_newtable(L);
        int j = 1;
    for (int i = 0; i < mons.size(); i++) {
        GameInst* mon = gs->get_instance(mons[i]);
        if (mon) {
            luawrap::push(L, mon);
            lua_rawseti(L, -2, j++);
        }
    }
    return 1;
}

static int gmap_enemies_list(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    CombatGameInst* actor = luawrap::get<CombatGameInst*>(L, 1);
    lua_newtable(L);
    int j = 1;
    for_all_enemies(gs->team_data(), actor, [&](CombatGameInst* actor) {
        luawrap::push(L, actor);
        lua_rawseti(L, -2, j++);
    });
    return 1;
}

static int gmap_allies_list(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    CombatGameInst* actor = luawrap::get<CombatGameInst*>(L, 1);
    lua_newtable(L);
    int j = 1;
    for_all_allies(gs->team_data(), actor, [&](CombatGameInst* actor) {
        luawrap::push(L, actor);
        lua_rawseti(L, -2, j++);
    });
    return 1;
}

static int gmap_players_on_team(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    team_id team = lua_tointeger(L, 1);
    lua_newtable(L);
    int j = 1;
    for_players_on_team(gs->team_data(), team, [&](PlayerInst* ally_player) {
        luawrap::push(L, ally_player);
        lua_rawseti(L, -2, j++);
    });
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
    Pos xy = luawrap::get<Pos>(L, 2);
    std::vector<PlayerDataEntry>& entries = gs->player_data().all_players();
    int max_dist = -1;
    for (int i = 0; i < entries.size(); i++) {
        GameInst* player = entries[i].player_inst.get();
        Pos dpos = xy - player->ipos();
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

static int gmap_object_visible(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    GameInst* inst = luawrap::get<GameInst*>(L, 1);
    Pos xy = stack_defaulted(L, 2, inst->ipos());
    PlayerInst* player = stack_defaulted(L, 3, (PlayerInst*)NULL);

    auto* prev_map = gs->get_level();
    auto* new_map = inst->get_map(gs);
    if (!new_map) {
        lua_pushboolean(L, false);
        return 1;
    }
    gs->set_level(new_map);

    lua_pushboolean(L, gs->radius_visible_test(xy.x, xy.y, inst->radius, player));
    gs->set_level(prev_map);
    return 1;
}

static int gmap_radius_visible(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    Pos xy = luawrap::get<Pos>(L, 1);
    double radius = lua_tonumber(L, 2);
    PlayerInst* player = stack_defaulted(L, 3, (PlayerInst*)NULL);
    lua_pushboolean(L, gs->radius_visible_test(xy.x, xy.y, radius, player));
    return 1;
}

static int gmap_object_solid_check(lua_State* L) {
    int nargs = lua_gettop(L);
    GameState* gs = lua_api::gamestate(L);
    GameInst* inst = luawrap::get<GameInst*>(L, 1);
    Pos p = stack_defaulted(L, 2, inst->ipos());

    lua_pushboolean(L, inst->get_map(gs)->solid_test(inst, p.x, p.y));
    return 1;
}

static int gmap_set_vision_radius(LuaStackValue map_obj, int vision_radius) {
    GameState* gs = lua_api::gamestate(map_obj);
    level_id map_id = mapid(map_obj);
    LANARTS_ASSERT(vision_radius > 0 && vision_radius <= 10);
    gs->game_world().get_level(map_id)->vision_radius() = vision_radius;
    return 0;
}

const int MAX_RET = 64;

static int gmap_object_collision_check(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    GameInst* inst = luawrap::get<GameInst*>(L, 1);
    Pos xy = stack_defaulted(L, 2, inst->ipos());
    GameInst* objects[MAX_RET];

    int nret = gs->object_radius_test(inst, objects, MAX_RET, NULL, xy.x, xy.y);
    lua_newtable(L);
    for (int i = 0; i < nret; i++){
        luawrap::push(L, objects[i]);
        lua_rawseti(L, -2, i+1);
    }
    return 1;
}

static int gmap_radius_collision_check(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    GameInst* objects[MAX_RET];
    GameMapState* map = mapstate(LuaStackValue(L, 1));
    double radius = luawrap::get<double>(L, 2);
    Pos xy = luawrap::get<Pos>(L, 3);

    int nret = gs->object_radius_test(NULL, objects, MAX_RET, NULL, xy.x, xy.y, radius);
    lua_newtable(L);
    for (int i = 0; i < nret; i++){
        luawrap::push(L, objects[i]);
        lua_rawseti(L, -2, i+1);
    }
    return 1;
}

static std::vector<GameInst*> gmap_rectangle_collision_check(LuaStackValue map_obj, BBox area, LuaStackValue tester /* Can be empty */) {
    GameState* gs = lua_api::gamestate(map_obj);
    GameMapState* map = mapstate(map_obj);

    return map->game_inst_set().object_rectangle_test(area, tester.isnil() ? NULL : tester.as<GameInst*>());
}

static int gmap_object_tile_check(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    GameInst* inst = luawrap::get<GameInst*>(L, 1);
    GameMapState* new_level = gs->get_level(inst->current_floor);
    Pos pos = inst->ipos();
    if (lua_gettop(L) >= 2) {
        pos = luawrap::get<Pos>(L, 2);
    }

    Pos hit_pos;
    LEVEL_WRAPPED(
        bool collided = gs->tile_radius_test(pos.x, pos.y, inst->radius, true, -1, &hit_pos);
    );
    if (!collided) {
        lua_pushnil(L);
    } else {
        luawrap::push(L, hit_pos);
    }
    return 1;
}

static int gmap_radius_tile_check(lua_State* L) {
    GameState* gs = lua_api::gamestate(L);
    GameTiles& tiles = mapstate(LuaStackValue(L, 1), true)->tiles();
    Pos pos = luawrap::get<Pos>(L, 2);
    int radius = lua_tointeger(L, 3);

    Pos hit_pos;
    bool collided = tiles.radius_test(pos, radius, true, -1, &hit_pos);
    if (!collided) {
        lua_pushnil(L);
    } else {
        luawrap::push(L, hit_pos);
    }
    return 1;
}

static LuaStackValue gmap_line_tile_check(LuaStackValue map_obj, Pos from_xy, Pos to_xy) {
    lua_State* L = map_obj.luastate();
    GameState* gs = lua_api::gamestate(L);
    GameMapState* prev_map = gs->get_level();
    gs->set_level(mapstate(map_obj));

    Pos hit_pos;
    bool collided = gs->tile_line_test(from_xy, to_xy, true, -1, &hit_pos);
    if (!collided) {
        lua_pushnil(L);
    } else {
        luawrap::push(L, hit_pos);
    }
    gs->set_level(prev_map);

    return LuaStackValue(L, -1);
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


static void gmap_transfer(LuaStackValue linst, LuaStackValue map_obj, Pos xy) {
    GameState* gs = lua_api::gamestate(linst);
    auto* inst = linst.as<GameInst*>();
    gs->game_world().level_move(inst->id, xy.x, xy.y, inst->current_floor, mapid(map_obj));
}

static void gmap_map_step(LuaStackValue table) {
    GameState* gs = lua_api::gamestate(table);
    level_id map_id = mapid(table["map"]);
    bool simulate_monsters = luawrap::defaulted(table, "simulate_monsters", true);
    gs->game_world().get_level(map_id)->step(gs, simulate_monsters);
}

static void gmap_map_draw(LuaStackValue table) {
    GameState* gs = lua_api::gamestate(table);
    level_id map_id = mapid(table["map"]);
    bool reveal_all = luawrap::defaulted(table["reveal_all"], false);
    gs->game_world().get_level(map_id)->draw(gs, reveal_all);
}

namespace lua_api {
    void register_lua_core_GameMap(lua_State* L) {
        LuaValue gmap = register_lua_submodule(L, "core.Map");
        gmap["create"].bind_function(gmap_create);
        gmap["transfer"].bind_function(gmap_transfer);
        gmap["TILE_SIZE"] = TILE_SIZE;

        // Query functions:
        gmap["lookup"].bind_function(gmap_lookup);

        gmap["objects_list"].bind_function(gmap_objects_list);

        gmap["tile_is_solid"].bind_function(gmap_tile_is_solid);
        gmap["tile_is_seethrough"].bind_function(gmap_tile_is_seethrough);
        gmap["tile_set_solid"].bind_function(gmap_tile_set_solid);
        gmap["tile_set_seethrough"].bind_function(gmap_tile_set_seethrough);
        gmap["tile_was_seen"].bind_function(gmap_tile_was_seen);

        gmap["tiles_predraw"].bind_function(gmap_tiles_predraw);
        gmap["tiles_postdraw"].bind_function(gmap_tiles_postdraw);
        gmap["instances_draw"].bind_function(gmap_instances_draw);

        gmap["add_object"].bind_function(gmap_add_object);
        gmap["instance"].bind_function(gmap_instance);

        gmap["monsters_list"].bind_function(gmap_monsters_list);
        gmap["players_list"].bind_function(gmap_players_list);
        gmap["allies_list"].bind_function(gmap_allies_list);
        gmap["players_on_team"].bind_function(gmap_players_on_team);
        gmap["enemies_list"].bind_function(gmap_enemies_list);
        gmap["monsters_seen"].bind_function(gmap_monsters_seen);
        gmap["monsters"].bind_function(gmap_monsters);

        gmap["map_step"].bind_function(gmap_map_step);
        gmap["map_label"].bind_function(gmap_map_label);
        gmap["map_draw"].bind_function(gmap_map_draw);

        gmap["distance_to_player"].bind_function(gmap_distance_to_player);

        gmap["object_visible"].bind_function(gmap_object_visible);
        gmap["radius_visible"].bind_function(gmap_radius_visible);

        gmap["object_collision_check"].bind_function(gmap_object_collision_check);
        gmap["radius_collision_check"].bind_function(gmap_radius_collision_check);
        gmap["object_tile_check"].bind_function(gmap_object_tile_check);
        gmap["radius_tile_check"].bind_function(gmap_radius_tile_check);
        gmap["rectangle_collision_check"].bind_function(gmap_rectangle_collision_check);
        gmap["line_tile_check"].bind_function(gmap_line_tile_check);

        gmap["object_solid_check"].bind_function(gmap_object_solid_check);
        gmap["set_vision_radius"].bind_function(gmap_set_vision_radius);

        gmap["place_free"].bind_function(gmap_place_free);
        gmap["radius_place_free"].bind_function(gmap_radius_place_free);
        gmap["maptype"].bind_function(gmap_maptype);
    }
}
