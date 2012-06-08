/*
 * combat_logic.cpp
 *  Represents formulas used in combat
 */

#include <cmath>

#include "combat_logic.h"

/* What power, resistance difference causes damage to be raised by 100% */
const int POWER_MULTIPLE_INTERVAL = 10;

static float damage_multiplier(const DerivedStats& attacker,
		const DerivedStats& defender) {
	float powdiff = attacker.power - defender.resistance;
	float intervals = powdiff / POWER_MULTIPLE_INTERVAL;
	if (intervals < 0) {

	} else {
		//100% + 100% * intervals
		return 1.0f + intervals;
	}
}

static int damage_formula(const DerivedStats& attacker,
		const DerivedStats& defender) {
	float mult = damage_multiplier(attacker, defender);
	int base = attacker.damage - defender.reduction;
	if (base < 0)
		return 0;
	return round(mult * base);
}

int magic_damage_formula(const EffectiveStats& attacker,
		const EffectiveStats& defender) {
	return damage_formula(attacker.magic, defender.magic);
}

int physical_damage_formula(const EffectiveStats& attacker,
		const EffectiveStats& defender) {
	return damage_formula(attacker.physical, defender.physical);
}

EffectiveStats derive_stats(const CoreStats& stats) {

}

