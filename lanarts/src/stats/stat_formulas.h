/*
 * stat_formulas.h
 *  Represents formulas used in combat
 */

#ifndef STAT_FORMULAS_H_
#define STAT_FORMULAS_H_

struct lua_State;
class Attack;
struct CombatStats;
struct EffectiveAttackStats;
struct EffectiveStats;
class CombatGameInst;
class GameState;

int damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender);

EffectiveStats effective_stats(GameState* gs, CombatGameInst* inst, const CombatStats& stats);
int experience_needed_formula(int xplevel);

#endif /* STAT_FORMULAS_H_ */
