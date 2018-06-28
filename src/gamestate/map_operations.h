#pragma once

#include "GameState.h"
#include "GameMapState.h"
#include "objects/PlayerInst.h"

void reveal_map(GameState* gs, GameMapState* level);
inline void map_completion(GameState* gs, PlayerInst* player, GameMapState* level) {
    level->for_each<EnemyInst>([&](EnemyInst* inst) {
        if (are_enemies(player, inst)) {
            inst->damage(gs, 100000, player);
        }
        return true;
    });
}

inline bool map_completion(GameState* gs, PlayerInst* player, const char* map_name) {
    auto& world = gs->game_world();
    return !world.for_each([&](GameMapState* level) {
        if (level->label() == map_name) {
            map_completion(gs, player, level);
            reveal_map(gs, level);
            return true;
        }
        return false;
    });
}