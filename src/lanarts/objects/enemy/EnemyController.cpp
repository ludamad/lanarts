/*
 * EnemyController.cpp:
 *  Centralized location of all pathing decisions of enemies, with collision avoidance
 */

#include <algorithm>
#include <cmath>

#include <rvo2/RVO.h>

#include "../../display/colour_constants.h"
#include "../../display/tile_data.h"

#include "../../gamestate/GameState.h"
#include "../../gamestate/PlayerData.h"

#include "../../stats/items/WeaponEntry.h"

#include "../../util/math_util.h"

#include "../player/PlayerInst.h"

#include "../collision_filters.h"
#include "EnemyController.h"

#include "EnemyInst.h"

const int PATHING_RADIUS = 500;
const int HUGE_DISTANCE = 1000000;
