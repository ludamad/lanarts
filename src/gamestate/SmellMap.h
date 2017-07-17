#ifndef SMELLMAP_H
#define SMELLMAP_H

#include <vector>

#include "collision_avoidance/CollisionAvoidance.h"

#include "pathfind/AStarPath.h"

#include "pathfind/FloodFillPaths.h"
#include <lcommon/SerializeBuffer.h>

#include "objects/GameInst.h"
#include "objects/EnemyInst.h"

#include "gamestate/GameTiles.h"

class SmellMap {
public:
    SmellMap(GameTiles& tiles);
    void step();
    void add_smell(GameInst* inst, float smell);
    PosF towards_least_smell(CombatGameInst* inst);
    PosF towards_most_smell(CombatGameInst* inst);
private:
    GameTiles* tiles; // Should not outlive level!
    Grid<float> map;
};

#endif
