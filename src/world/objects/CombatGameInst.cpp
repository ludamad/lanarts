/*
 * CombatGameInst.cpp:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#include "../GameState.h"

#include "CombatGameInst.h"

bool CombatGameInst::damage(GameState* gs, int dmg) {
	if (stats.core.hurt(dmg)){
		gs->remove_instance(this);
		return true;
	}
	return false;
}

void CombatGameInst::step(GameState* gs) {
	stats.cooldowns.step();
}

