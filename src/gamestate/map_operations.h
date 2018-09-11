#pragma once

#include <iostream>
#include <objects/FeatureInst.h>
#include "GameState.h"
#include "GameMapState.h"
#include "objects/PlayerInst.h"
#include "objects/ItemInst.h"

void reveal_map(GameState* gs, GameMapState* level);
inline void map_completion(GameState* gs, PlayerInst* player, GameMapState* map) {
    gs->do_with_map(map, [&]() -> void {
        map->for_each<EnemyInst>([&](EnemyInst* inst) {
            if (are_enemies(player, inst)) {
                inst->damage(gs, 100000, player);
            }
            return true;
        });
        map->for_each<ItemInst>([&](ItemInst* inst) {
            GameAction action;
            action.use_id = inst->id;
            player->pickup_item(gs, action);
            return true;
        });
        // TODO should FeatureInst be renamed to PortalInst?
        map->for_each<FeatureInst>([&](FeatureInst* inst) {
            if (!inst->has_been_used()) {
                inst->player_interact(gs, player);
                return false;
            }
            return true;
        });
    });
}

inline bool map_completion(GameState* gs, PlayerInst* player, const char* map_name) {
    auto& world = gs->game_world();
    GameMapState* map = gs->get_map(map_name);
    if (map != nullptr) {
        map_completion(gs, player, map);
        reveal_map(gs, map);
        return true;
    }
    return false;
}

GameMapState* mapstate(LuaStackValue map_obj, bool defaulted = false);

inline void visit_all_maps(GameState* gs, PlayerInst* player) {
    auto& world = gs->game_world();
    std::set<FeatureInst*> used_portals;
    int previous_portal_size = -1; // Used to force first iteration
    while (int(used_portals.size()) > previous_portal_size) {
        previous_portal_size = (int)used_portals.size();
        std::vector<GameMapState*> maps_copy = world.maps();
        for (auto* map : maps_copy) {
            map->for_each<FeatureInst>([&](FeatureInst* inst) {
                gs->do_with_map(map, [&](){
                    bool new_portal = used_portals.find(inst) == used_portals.end();
                    if (new_portal) {
                        inst->player_interact(gs, player);
                        used_portals.insert(inst);
                    }
                });
                return true;
            });
        }
    }
    world.for_each([&](GameMapState* level) {
       std::cout << "L: " << level->label() << std::endl;
       return true;
    });
}


inline void use_portal_between_maps(GameState* gs, PlayerInst* player, const char* start_map_label,
                                    const char* end_map_label) {
    LuaValue map_module = lua_api::import(gs->luastate(), "core.Map");
    LuaValue original_transfer = map_module["transfer"];

    auto* start_map = gs->get_map(start_map_label);
    auto* end_map = gs->get_map(end_map_label);
    if (!start_map) {
        return;
    }

    std::vector<GameMapState*> maps_copy = gs->game_world().maps();
    start_map->for_each<FeatureInst>([&](FeatureInst* inst) {
        bool continue_iteration = true;
        // Use 'transfer' to grab the map this takes us to
        map_module["transfer"] = [&](LuaStackValue inst, LuaStackValue map_obj, Pos xy) -> void {
            if (mapstate(map_obj) == end_map) {
                gs->do_with_map(start_map, [&](){
                    lcall(original_transfer, inst, map_obj, xy);
                });
                continue_iteration = false;
            }
        };
        return continue_iteration;
    });
    map_module["transfer"] = original_transfer;
}