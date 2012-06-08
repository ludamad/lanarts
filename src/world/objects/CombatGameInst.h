/*
 * CombatGameInst.h:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#ifndef COMBATGAMEINST_H_
#define COMBATGAMEINST_H_

#include "../../gamestats/stats.h"

#include "../../pathfind/pathfind.h"

#include "GameInst.h"

class CombatGameInst: public GameInst {
public:
	CombatGameInst(int x, int y, int radius, bool solid = true, int depth = 0) :
			GameInst(x, y, radius, solid, depth) {
	}

	CombatStats& base_stats() {
		return stats;
	}
	EffectiveStats& effective_stats() {
		return estats;
	}

	bool damage(GameState* gs, int dmg);

	void step(GameState* gs);

protected:
	PathInfo path;
	CombatStats stats;
private:
	EffectiveStats estats;
};

#endif /* COMBATGAMEINST_H_ */
