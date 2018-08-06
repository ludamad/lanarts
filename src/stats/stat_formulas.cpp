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
	if (powdiff < 0) {
            return 1.0f / (1.0f - intervals);
	} else {
            //100% + 100% * intervals
            return 1.0f + intervals;
	}
}

// This stat formula is a lot simpler than what it originally was.
// Simpler ends up being easier to understand and balance.
// Now, there is just a base damage, and a multiplier based on the defenders resistance and the attacker's power.
// This is later multiplied by type resistances / strengths.
static float basic_damage_formula(const EffectiveAttackStats& attacker, float resistance) {
	float mult = damage_multiplier(attacker.power, resistance) * attacker.type_multiplier;
	float result = attacker.damage * mult;
	event_log("basic_damage_formula: mult=%.2f, damage=%.2f defence=%.2f result=%.2f",
			mult, attacker.damage, resistance, result);
	return std::max(0.0f, result);
}

static float physical_damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender) {
	return basic_damage_formula(attacker, defender.core.defence);
}

static float magic_damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender) {
	return basic_damage_formula(attacker, defender.core.willpower);
}

float damage_formula(const EffectiveAttackStats& attacker,
		const EffectiveStats& defender) {
	float mdmg = magic_damage_formula(attacker, defender);
	float pdmg = physical_damage_formula(attacker, defender);

	event_log("damage_formula attacker damage=%.2f power=%.2f cooldown=%.2f magic_percentage=%.2f",
			attacker.damage, attacker.power, attacker.cooldown,
			attacker.magic_percentage);
	event_log("damage_formula defender physical.resistance=%.2f magic.resistance=%.2f",
			defender.core.defence,
			defender.core.willpower);
	event_log("damage_formula: mdmg=%f, pdmg=%f", mdmg, pdmg);
	event_log("damage_formula: type modifier=%f", attacker.type_multiplier);

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

	core.defence += entry.resistance().calculate(mt, core);
	core.willpower += entry.magic_resistance().calculate(mt, core);

	// Factor in spells granted from equipment:
	for (int i = 0; i < entry.spells_granted.amount(); i++) {
		int spell = entry.spells_granted.get(i);
		if (!effective.spells.has_spell(spell)) {
			effective.spells.add_spell(spell);
		}
	}
	// Factor in dynamic bonuses gained from equipment:

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
	ret.cooldown_modifiers.apply(ret.cooldown_mult);
	event_log("effective_stats: hp=%d, mp=%d, hpregen=%f, mpregen=%f",
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

