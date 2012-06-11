/*
 * combat_logic.h:
 *  Represents movement and attack decisions for an entity
 */

#ifndef COMBAT_LOGIC_H_
#define COMBAT_LOGIC_H_

class GameState;
class CombatGameInst;

void combat_ai_decisions(GameState* gs, CombatGameInst* inst);

#endif /* COMBAT_LOGIC_H_ */
