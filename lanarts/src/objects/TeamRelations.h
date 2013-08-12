/*
 * TeamRelations.h:
 *  Bridge to lua-defined team relationships.
 *  Fairly hardcoded & inefficient for now.
 */

#ifndef TEAMRELATIONS_H_
#define TEAMRELATIONS_H_

class CombatGameInst;

bool teamrelations_is_hostile(CombatGameInst* inst1, CombatGameInst* inst2);
bool teamrelations_is_friendly(CombatGameInst* inst1, CombatGameInst* inst2);
bool teamrelations_is_neutral(CombatGameInst* inst1, CombatGameInst* inst2);

#endif /* TEAMRELATIONS_H_ */
