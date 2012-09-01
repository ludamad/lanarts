/*
 * CombatGameInstFunctions.h:
 *  Utility functions that operate on multiple CombatGameInst's
 */

#ifndef COMBATGAMEINSTFUNCTIONS_H_
#define COMBATGAMEINSTFUNCTIONS_H_

#include <vector>

class CombatGameInst;

bool inst_can_see(CombatGameInst* inst1, CombatGameInst* inst2);
float inst_distance(CombatGameInst* inst1, CombatGameInst* inst2);

/* Provides the best (lower bound) estimate possible cheaply */
float estimate_inst_path_distance(CombatGameInst* inst1, CombatGameInst* inst2);

CombatGameInst* find_closest_hostile(GameState* gs, CombatGameInst* inst,
		const std::vector<CombatGameInst*>& candidates);

#endif /* COMBATGAMEINSTFUNCTIONS_H_ */
