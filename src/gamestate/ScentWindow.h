#ifndef SCENTWINDOW_H_
#define SCENTWINDOW_H_

#include <vector>

#include "collision_avoidance/CollisionAvoidance.h"

#include "pathfind/AStarPath.h"

#include "pathfind/FloodFillPaths.h"
#include <lcommon/SerializeBuffer.h>

#include "objects/GameInst.h"
#include "objects/EnemyInst.h"

#include "gamestate/GameTiles.h"
#include "ScentWindow.h"

class ScentWindow {
public:
    ScentWindow(BBox area);
    void add_smell(Pos xy, float smell);
    float get(Pos xy) const; // returns 0 if out of bounds
    BBox area;
private:
    Grid<float> map;
};

std::tuple<float, PosF> towards_least_smell(GameState* gs, ScentWindow& window, CombatGameInst* inst);
std::tuple<float, PosF> towards_most_smell(GameState* gs, ScentWindow& window, CombatGameInst* inst);

void compute_diffusion(const ScentWindow& input, ScentWindow& output);

#endif