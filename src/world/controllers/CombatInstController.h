/*
 * CombatInstController.h:
 *  Controls behaviour of CombatGameInst's
 */

#ifndef COMBATINSTCONTROLLER_H_
#define COMBATINSTCONTROLLER_H_

#include "../../util/game_basic_structs.h"

class CombatGameInst;

struct InstWithTarget {
	CombatGameInst* inst;
	CombatGameInst* target;
	InstWithTarget(CombatGameInst* inst, CombatGameInst* target) :
			inst(inst), target(target) {
	}
};

namespace RVO {
struct RVOSimulator;
}

class CombatInstController {
public:
	CombatInstController();
private:
	RVO::RVOSimulator* monster_simulator;
	RVO::RVOSimulator* player_simulator;
};

#endif /* COMBATINSTCONTROLLER_H_ */
