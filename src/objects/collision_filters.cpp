/*
 * collision_filters.cpp:
 *  Defines functions that are often used in conjunction with collision detection
 */

#include "enemy/EnemyInst.h"
#include "FeatureInst.h"
#include "player/PlayerInst.h"
#include "GameInst.h"
#include "ItemInst.h"
#include "collision_filters.h"

bool item_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<ItemInst*>(other) != NULL;
}

bool enemy_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<EnemyInst*>(other) != NULL;
}

bool player_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<PlayerInst*>(other) != NULL;
}

bool feature_colfilter(GameInst* self, GameInst* other) {
    return dynamic_cast<FeatureInst*>(other) != NULL;
}
