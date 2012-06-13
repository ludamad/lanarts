/*
 * stat_formulas.cpp
 *  Represents formulas used in combat
 */

#include <cmath>

#include "../data/weapon_data.h"

#include "../world/GameState.h"

#include "combat_stats.h"
#include "stat_formulas.h"
#include "stats.h"

/* What power, resistance difference causes damage to be raised by 100% */
const int POWER_MULTIPLE_INTERVAL = 30;

static float damage_multiplier(const DerivedStats& attacker,
		const DerivedStats& defender) {
	float powdiff = attacker.power - defender.resistance;
	float intervals = powdiff / POWER_MULTIPLE_INTERVAL;
	if (intervals < 0) {
		//100% / (1+intervals)
		return 1.0f / (1.0f - intervals);
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

int physical_damage_formula(const EffectiveStats& attacker,
		const EffectiveStats& defender) {
	return damage_formula(attacker.physical, defender.physical);
}

int magic_damage_formula(const EffectiveStats& attacker,
		const EffectiveStats& defender) {
	return damage_formula(attacker.magic, defender.magic);
}

static void derive_from_equipment(MTwist& mt, EffectiveStats& effective,
		const _Equipment& equipment) {
	CoreStats& core = effective.core;
	WeaponEntry& wentry = equipment.weapon.weapon_entry();
	effective.physical.damage = wentry.damage.calculate(mt, core);
	effective.physical.power = wentry.power.calculate(mt, core);
	if (equipment.projectile.valid_projectile()){
		ProjectileEntry& pentry = equipment.projectile.projectile_entry();
		effective.physical.damage += pentry.damage.calculate(mt, core);
		effective.physical.power += pentry.power.calculate(mt, core);
	}
	effective.physical.resistance = core.defence;
	effective.magic.damage = core.magic;
	effective.magic.resistance = core.willpower;
}

EffectiveStats effective_stats(GameState* gs, const CombatStats& stats) {
	lua_State* L = gs->get_luastate();
	EffectiveStats ret;
	ret.core = stats.core;
	ret.movespeed = stats.movespeed;
	derive_from_equipment(gs->rng(), ret, stats.equipment);
	stats.effects.process(L, stats, ret);
	return ret;
}

int experience_needed_formula(int xplevel) {
	float proportion = pow(xplevel, 1.75);
	return round(proportion) * 50 + 100;
}

