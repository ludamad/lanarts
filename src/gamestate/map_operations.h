#pragma once

#include <iostream>
#include <objects/FeatureInst.h>
#include "GameState.h"
#include "GameMapState.h"
#include "objects/PlayerInst.h"
#include "objects/ItemInst.h"

void reveal_map(GameState* gs, GameMapState* level);
inline void map_completion(GameState* gs, PlayerInst* player, GameMapState* level) {
    gs->do_with_map(level, [&]() {
        level->for_each<EnemyInst>([&](EnemyInst* inst) {
            if (are_enemies(player, inst)) {
                inst->damage(gs, 100000, player);
            }
            return true;
        });
        level->for_each<ItemInst>([&](ItemInst* inst) {
            GameAction action;
            action.use_id = inst->id;
            player->pickup_item(gs, action);
            return true;
        });
        // TODO should FeatureInst be renamed to PortalInst?
        level->for_each<FeatureInst>([&](FeatureInst* inst) {
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
    return !world.for_each([&](GameMapState* level) {
        if (level->label() == map_name) {
            map_completion(gs, player, level);
            reveal_map(gs, level);
            return false;
        }
        return true;
    });
}
