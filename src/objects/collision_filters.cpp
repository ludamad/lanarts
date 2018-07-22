/*
 * collision_filters.cpp:
 *  Defines functions that are often used in conjunction with collision detection
 */

#include "EnemyInst.h"
#include "FeatureInst.h"
#include "PlayerInst.h"
#include "GameInst.h"
#include "ItemInst.h"
#include "collision_filters.h"

#include "stats/items/ItemEntry.h"

bool any_colfilter(GameInst* self, GameInst* other) {
    return true;
}

bool item_colfilter(GameInst* self, GameInst* other) {
	return dynamic_cast<ItemInst*>(other) != NULL;
}

bool autopickup_colfilter(GameInst* self, GameInst* other) {
    ItemInst* i = NULL;
    if ( (i = dynamic_cast<ItemInst*>(other)) ) {
        Item& item = i->item_type();
        ItemEntry& entry = item.item_entry();
        if (entry.stackable) {
            return true;
        }
    }
    return false;
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