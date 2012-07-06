/*
 * CombatInstController.h:
 *  Controls behaviour of CombatGameInst's
 */

#ifndef COMBATINSTCONTROLLER_H_
#define COMBATINSTCONTROLLER_H_

#include <vector>

#include "../../util/game_basic_structs.h"

class CombatGameInst;
class GameState;

struct InstWithTarget {
	CombatGameInst* inst;
	CombatGameInst* target;
	InstWithTarget(CombatGameInst* inst, CombatGameInst* target) :
			inst(inst), target(target) {
	}
};

//Defines a group of enemies that sticks close together
struct CombatMob {
	// Designated leader, used for arbitration
	obj_id leader;
	// Other members, includes leader for convenience
	std::vector<obj_id> member_ids;
};

namespace RVO {
struct RVOSimulator;
}

//One instance per level
class CombatInstController {
public:
	CombatInstController();

	/*Players in current level are kept track of for efficiency in retrieval */
	void register_player(obj_id player);
	void deregister_player(obj_id player);

	const std::vector<obj_id>& player_ids() {
		return _player_ids;
	}

	void update_player_fields_of_view(GameState* gs);

private:
	std::vector<obj_id> _player_ids;

	RVO::RVOSimulator* monster_simulator;
	RVO::RVOSimulator* player_simulator;
};

#endif /* COMBATINSTCONTROLLER_H_ */
