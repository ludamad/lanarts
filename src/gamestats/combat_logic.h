/*
 * combat_logic.h
 *  Represents formulas used in combat
 */

#ifndef COMBAT_LOGIC_H_
#define COMBAT_LOGIC_H_

#include "stats.h"

int magic_damage_formula(const EffectiveStats& attacker,
		const EffectiveStats& defender);
int physical_damage_formula(const EffectiveStats& attacker,
		const EffectiveStats& defender);

EffectiveStats derive_stats(const CoreStats& stats);

#endif /* COMBAT_LOGIC_H_ */
