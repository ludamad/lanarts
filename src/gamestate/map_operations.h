#pragma once

#include <iostream>
#include <objects/FeatureInst.h>
#include "GameState.h"
#include "GameMapState.h"
#include "objects/PlayerInst.h"
#include "objects/ItemInst.h"

void reveal_map(GameState* gs, GameMapState* level);
inline void map_completion(GameState* gs, PlayerInst* player, GameMapState* map) {
    gs->do_with_map(map, [&]() {
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
    return !world.for_each([&](GameMapState* map) {
        if (map->label() == map_name) {
            map_completion(gs, player, map);
            reveal_map(gs, map);
            return false;
        }
        return true;
    });
}

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