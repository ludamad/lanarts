/*
 * EquipmentEntry.h:
 *  Represents an equippable item.
 */

#ifndef EQUIPMENTENTRY_H_
#define EQUIPMENTENTRY_H_

#include "ItemEntry.h"

#include "../../lanarts_defines.h"

struct DamageModifiers {
	CoreStatMultiplier power, damage;
	float magic_percentage, physical_percentage;
	float resistability;
	DamageModifiers() :
			magic_percentage(0.0f), physical_percentage(0.0f), resistability(
					1.0f) {
	}
};

struct DefenceModifiers {
	CoreStatMultiplier resistance, damage_reduction;
	CoreStatMultiplier magic_resistance, magic_reduction;
};

//Stat modifiers that trivially stack
struct StatModifiers {
	// additive
	CoreStats core_mod;
	DamageModifiers damage_mod;
	DefenceModifiers defence_mod;
	// multiplicative
	float spell_cooldown_mult, melee_cooldown_mult;
	StatModifiers() :
			spell_cooldown_mult(0.0f), melee_cooldown_mult(0.0f) {
	}
};

struct StatusEffectModifiers {
	std::vector<effect_id> status_effects;
};

class EquipmentEntry: public ItemEntry {
public:
	enum equip_type {
		NONE, WEAPON, PROJECTILE, ARMOUR
	};
	virtual ~EquipmentEntry() {
	}
	// Stat modifiers while wearing this equipment
	StatModifiers stat_modifiers;
	// Status effects from wearing this armour
	StatusEffectModifiers effect_modifiers;
};

#endif /* EQUIPMENTENTRY_H_ */
