/*
 * InstTypeEnum.h:
 *  Defines a simple enum for serializing arbitrary GameInst objects
 */

#ifndef INSTTYPEENUM_H_
#define INSTTYPEENUM_H_

#include "enemy/EnemyInst.h"
#include "player/PlayerInst.h"
#include "store/StoreInst.h"
#include "AnimatedInst.h"
#include "FeatureInst.h"
#include "ItemInst.h"
#include "ProjectileInst.h"

enum InstType {
	ENEMY_INST,
	PLAYER_INST,
	STORE_INST,
	ANIMATED_INST,
	FEATURE_INST,
	ITEM_INST,
	PROJECTILE_INST,
	INVALID_INST
};

inline InstType get_inst_type(GameInst* inst) {
	if (dynamic_cast<EnemyInst*>(inst)) {
		return ENEMY_INST;
	} else if (dynamic_cast<PlayerInst*>(inst)) {
		return PLAYER_INST;
	} else if (dynamic_cast<StoreInst*>(inst)) {
		return STORE_INST;
	} else if (dynamic_cast<AnimatedInst*>(inst)) {
		return ANIMATED_INST;
	} else if (dynamic_cast<FeatureInst*>(inst)) {
		return FEATURE_INST;
	} else if (dynamic_cast<ItemInst*>(inst)) {
		return ITEM_INST;
	} else if (dynamic_cast<ProjectileInst*>(inst)) {
		return PROJECTILE_INST;
	}
	LANARTS_ASSERT(false);
	return INVALID_INST;
}

inline bool is_inst_type(GameInst* inst, InstType type) {
	switch (type) {
	case ENEMY_INST:
		return dynamic_cast<EnemyInst*>(inst) != NULL;
	case PLAYER_INST:
		return dynamic_cast<PlayerInst*>(inst) != NULL;
	case STORE_INST:
		return dynamic_cast<StoreInst*>(inst) != NULL;
	case ANIMATED_INST:
		return dynamic_cast<AnimatedInst*>(inst) != NULL;
	case FEATURE_INST:
		return dynamic_cast<FeatureInst*>(inst) != NULL;
	case ITEM_INST:
		return dynamic_cast<ItemInst*>(inst) != NULL;
	case PROJECTILE_INST:
		return dynamic_cast<ProjectileInst*>(inst) != NULL;
	}
	LANARTS_ASSERT(false);
	return false;
}

inline GameInst* from_inst_type(InstType type) {
	switch (type) {
	case ENEMY_INST:
		return new EnemyInst(NONE, NONE, NONE, NONE);
	case PLAYER_INST:
		return new PlayerInst();
	case STORE_INST:
		return new StoreInst(Pos(), NONE, NONE, StoreInventory());
	case ANIMATED_INST:
		return new AnimatedInst(Pos(), NONE);
	case FEATURE_INST:
		return new FeatureInst(Pos(), FeatureInst::feature_t(NONE), false);
	case ITEM_INST:
		return new ItemInst(Item(), Pos());
	case PROJECTILE_INST:
		return new ProjectileInst(Projectile(), EffectiveAttackStats(), NONE,
				Pos(), Pos(), NONE, NONE);
	}
	LANARTS_ASSERT(false);
	return NULL;
}

#endif /* INSTTYPEENUM_H_ */
