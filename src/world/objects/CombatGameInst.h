/*
 * CombatGameInst.h:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#ifndef COMBATGAMEINST_H_
#define COMBATGAMEINST_H_

#include "../../gamestats/stats.h"

class CombatGameInst: public GameInst {
public:
	CombatGameInst(int x, int y, int radius, bool solid = true, int depth = 0) :
			GameInst(x, y, radius, solid, depth) {
	}
	EffectiveStats& effective_stats() {
		return estats;
	}
	void damage(int dmg);

protected:
	CombatStats stats;
private:
	EffectiveStats estats;
};

#endif /* COMBATGAMEINST_H_ */
