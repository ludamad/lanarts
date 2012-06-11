/*
 * movement_logic.h:
 *  Represents pathfinding and movement decisions
 */

#ifndef MOVEMENT_LOGIC_H_
#define MOVEMENT_LOGIC_H_

class GameState;
class CombatGameInst;

void movement_ai_decisions(GameState* gs, CombatGameInst* inst);

#endif /* MOVEMENT_LOGIC_H_ */
