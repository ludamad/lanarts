/*
 * attack_logic.h:
 *  Represents melee and spell decisions for an entity
 */

#ifndef ATTACK_LOGIC_H_
#define ATTACK_LOGIC_H_

struct AttackStats;
class GameState;
class CombatGameInst;

/* Return whether 'attack' was chosen*/
bool attack_ai_choice(GameState* gs, CombatGameInst* inst, CombatGameInst* target, AttackStats& attack);

#endif /* ATTACK_LOGIC_H_ */
