/*
 * stat_formulas.cpp
 *  Represents formulas used in combat
 */

#include <cmath>
#include <algorithm>

#include "gamestate/GameState.h"
#include "gamestate/GameLogger.h"

#include "items/EquipmentEntry.h"
#include "items/WeaponEntry.h"
#include "objects/CombatGameInst.h"

#include "combat_stats.h"

#include "stat_formulas.h"
#include "stats.h"
#include "ClassEntry.h"

/* What power, resistance difference causes damage to be raised by 100% */
const float POWER_MULTIPLE_INTERVAL = 20.0f;

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

static float basic_damage_formula(const EffectiveAttackStats& attacker,
		const DerivedStats& defender) {
        // TODO evaluate new change:
        // only apply multiplier to damage -- not defence. 
        // This will hopefully allow power/resistance to be more meaningful, 
        // as the multiplier will be applied to the larger number.
	float mult = damage_multiplier(attacker.power, defender.resistance);
	float result = attacker.damage * mult;
	event_log("basic_damage_formula: mult=%f, damage=%f defence=%f result=%f\n",
			mult, (float)attacker.damage, (float)defender.resistance, result);
        return std::max(0.0f, result);
}

static float physical_damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender) {
	return basic_damage_formula(attacker, defender.physical);
}

static float magic_damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender) {
	return basic_damage_formula(attacker, defender.magic);
}

float damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender) {
	float mdmg = magic_damage_formula(attacker, defender);
	float pdmg = physical_damage_formula(attacker, defender);

	event_log("damage_formula attacker damage=%d power=%d cooldown=%d magic_percentage=%f\n",
			attacker.damage, attacker.power, attacker.cooldown,
			attacker.magic_percentage);
	event_log("damage_formula defender physical.resistance=%f magic.resistance=%f\n",
			defender.physical.resistance,
			defender.magic.resistance);
	event_log("damage_formula: mdmg=%f, pdmg=%f\n", mdmg, pdmg);

	return mdmg * attacker.magic_percentage
			+ pdmg * attacker.physical_percentage();
}

static void factor_in_equipment_core_stats(MTwist& mt,
		EffectiveStats& effective, const Equipment& item) {
	CoreStats& core = effective.core;
	EquipmentEntry& entry = item.equipment_entry();

	core.apply_as_bonus(entry.core_stat_modifier());
}

static void factor_in_equipment_derived_stats(MTwist& mt,
		EffectiveStats& effective, const Equipment& item) {
	CoreStats& core = effective.core;
	EquipmentEntry& entry = item.equipment_entry();

	effective.cooldown_modifiers.apply(entry.cooldown_modifiers);

	effective.physical.resistance += entry.resistance().calculate(mt, core);
	effective.magic.resistance += entry.magic_resistance().calculate(mt, core);

	// Factor in spells granted from equipment:
	for (int i = 0; i < entry.spells_granted.amount(); i++) {
	    effective.spells.add_spell(entry.spells_granted.get(i));
	}
	// Factor in effects granted from equipment:
}

static void factor_in_equipment_stats(MTwist& mt, EffectiveStats& effective,
		const EquipmentStats& equipment) {
	const Inventory& inventory = equipment.inventory;
	for (int i = 0; i < inventory.max_size(); i++) {
		const ItemSlot& itemslot = inventory.get(i);
		if (itemslot.is_equipped()) {
			factor_in_equipment_core_stats(mt, effective, itemslot.item);
		}
	}
	for (int i = 0; i < inventory.max_size(); i++) {
		const ItemSlot& itemslot = inventory.get(i);
		if (itemslot.is_equipped()) {
			factor_in_equipment_derived_stats(mt, effective, itemslot.item);
		}
	}
}
static void derive_secondary_stats(MTwist& mt, EffectiveStats& effective) {
	CoreStats& core = effective.core;
	effective.physical.resistance += core.defence;
	effective.magic.resistance += core.willpower;
}

EffectiveStats effective_stats(GameState* gs, CombatGameInst* inst,
		const CombatStats& stats) {
	EffectiveStats ret;
	ret.core = stats.core;
	ret.movespeed = stats.movespeed;
	ret.spells = stats.spells;
    if (inst != NULL) {
        ret.allowed_actions = inst->effects.allowed_actions(gs);
        inst->effects.process(gs, inst, ret);
    }
	factor_in_equipment_stats(gs->rng(), ret, stats.equipment);
	derive_secondary_stats(gs->rng(), ret);
	ret.cooldown_modifiers.apply(ret.cooldown_mult);
	event_log("effective_stats: hp=%d, mp=%d, hpregen=%f, mpregen=%f\n",
			ret.core.hp, ret.core.mp, ret.core.hpregen, ret.core.mpregen);
	if (stats.class_stats.has_class()) {
	    if (stats.class_stats.class_entry().name == "Necromancer") {
	        ret.core.mpregen = 0;
	    }
	}
	return ret;
}

int experience_needed_formula(int xplevel) {
	float proportion = pow(xplevel, 2.5);
	return round(proportion) * 75 + 125;
}

