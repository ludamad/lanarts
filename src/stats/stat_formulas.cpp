/*
 * stat_formulas.cpp
 *  Represents formulas used in combat
 */

#include <cmath>

#include "../gamestate/GameState.h"
#include "armour_data.h"

#include "combat_stats.h"

#include "stat_formulas.h"
#include "stats.h"
#include "weapon_data.h"

/* What power, resistance difference causes damage to be raised by 100% */
const int POWER_MULTIPLE_INTERVAL = 50;

static float damage_multiplier(float power, float resistance) {
	float powdiff = power - resistance;
	float intervals = powdiff / POWER_MULTIPLE_INTERVAL;
	if (intervals < 0) {
		//100% / (1+intervals)
		return 1.0f / (1.0f - intervals);
	} else {
		//100% + 100% * intervals
		return 1.0f + intervals;
	}
}

static int basic_damage_formula(const EffectiveAttackStats& attacker,
		const DerivedStats& defender) {
	float mult = damage_multiplier(attacker.power, defender.resistance);
	float base = attacker.damage
			- defender.reduction * attacker.resist_modifier;
	if (base < 0)
		return 0;
	return round(mult * base);
}

static int physical_damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender) {
	return basic_damage_formula(attacker, defender.physical);
}

static int magic_damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender) {
	return basic_damage_formula(attacker, defender.magic);
}

int damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender) {
	float mdmg = magic_damage_formula(attacker, defender);
	float pdmg = physical_damage_formula(attacker, defender);

	return mdmg * attacker.magic_percentage
			+ pdmg * attacker.physical_percentage();
}

static void factor_in_armour_slot(MTwist& mt, EffectiveStats& effective,
		const _Armour& slot) {
	CoreStats& core = effective.core;
	_ArmourEntry& aentry = slot.armour_entry();

	effective.physical.resistance += aentry.resistance.calculate(mt, core);
	effective.magic.resistance += aentry.magic_resistance.calculate(mt, core);
	effective.physical.reduction += aentry.damage_reduction.calculate(mt, core);
	effective.magic.reduction += aentry.magic_reduction.calculate(mt, core);
}
static void factor_in_equipment(MTwist& mt, EffectiveStats& effective,
		const Equipment& equipment) {
	factor_in_armour_slot(mt, effective, equipment.armour);
}
static void derive_secondary_stats(MTwist& mt, EffectiveStats& effective) {
	CoreStats& core = effective.core;
	effective.physical.resistance += core.defence / 2.5f;
	effective.magic.resistance += core.willpower / 2.5f;
	effective.physical.reduction += core.defence / 2.0;
	effective.magic.reduction += core.willpower / 2.0;
}

EffectiveStats effective_stats(GameState* gs, CombatGameInst* inst,
		const CombatStats& stats) {
	EffectiveStats ret;
	ret.core = stats.core;
	ret.movespeed = stats.movespeed;
	ret.allowed_actions = stats.effects.allowed_actions(gs);
	stats.effects.process(gs, inst, ret);
	factor_in_equipment(gs->rng(), ret, stats.equipment);
	derive_secondary_stats(gs->rng(), ret);
	return ret;
}

int experience_needed_formula(int xplevel) {
	float proportion = pow(xplevel, 2.0);
	return round(proportion) * 75 + 50;
}

