/*
 * stat_formulas.h
 *  Represents formulas used in combat
 */

#ifndef STAT_FORMULAS_H_
#define STAT_FORMULAS_H_

struct lua_State;
struct Attack;
struct CombatStats;
struct EffectiveAttackStats;
struct EffectiveStats;
struct GameState;

int physical_damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender);
int magic_damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender);

EffectiveStats effective_stats(GameState* gs, const CombatStats& stats);
int experience_needed_formula(int xplevel);

#endif /* STAT_FORMULAS_H_ */
