/*
 * CombatInstController.h:
 *  Controls behaviour of CombatGameInst's
 */

#ifndef COMBATINSTCONTROLLER_H_
#define COMBATINSTCONTROLLER_H_

#include <vector>

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

	/*Players are kept track of for efficiency*/
	void register_player(obj_id player);
	void deregister_player(obj_id player);

	const std::vector<obj_id>& player_ids() {
		return _player_ids;
	}

private:
	std::vector<obj_id> _player_ids;

	RVO::RVOSimulator* monster_simulator;
	RVO::RVOSimulator* player_simulator;
};

#endif /* COMBATINSTCONTROLLER_H_ */
