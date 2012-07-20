/*
 * collision_filters.cpp:
 *  Defines functions that are often used in conjunction with collision detection
 */

#include "collision_filters.h"
#include "ItemInst.h"
#include "enemy/EnemyInst.h"
#include "GameInst.h"
#include "player/PlayerInst.h"

bool item_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<ItemInst*>(other) != NULL;
}

bool enemy_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<EnemyInst*>(other) != NULL;
}

bool player_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<PlayerInst*>(other) != NULL;
}
