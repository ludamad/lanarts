/*
 * EnemyController.cpp:
 *  Centralized location of all pathing decisions of enemies, with collision avoidance
 */

#include <algorithm>
#include <cmath>

#include <rvo2/RVO.h>

#include "../../gamestate/GameState.h"
#include "../../gamestate/PlayerData.h"

#include "EnemyInst.h"
#include "../player/PlayerInst.h"

#include "../../display/colour_constants.h"
#include "../../util/math_util.h"
#include "../collision_filters.h"

#include "../../display/tile_data.h"
#include "../../stats/weapon_data.h"

#include "EnemyController.h"

const int PATHING_RADIUS = 500;
const int HUGE_DISTANCE = 1000000;
