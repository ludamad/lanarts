/*
 * collision_util.cpp:
 *  Defines functions that are often used in conjunction with collision detection
 */

#include "collision_util.h"
#include "../../world/objects/ItemInst.h"
#include "../../world/objects/EnemyInst.h"
#include "../../world/objects/GameInst.h"
#include "../../world/objects/PlayerInst.h"

bool item_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<ItemInst*>(other) != NULL;
}

bool enemy_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<EnemyInst*>(other) != NULL;
}

bool player_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<PlayerInst*>(other) != NULL;
}
