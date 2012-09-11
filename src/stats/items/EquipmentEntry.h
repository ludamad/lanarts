/*
 * EquipmentEntry.h:
 *  Represents an equippable item.
 */

#ifndef EQUIPMENTENTRY_H_
#define EQUIPMENTENTRY_H_

#include "../../lanarts_defines.h"

#include "../stat_modifiers.h"

#include "ItemEntry.h"

class EquipmentEntry: public ItemEntry {
public:
	enum equip_type {
		NONE, WEAPON, PROJECTILE, BODY_ARMOUR, RING, BOOTS, GLOVES, HELMET
	};
	EquipmentEntry(equip_type type = NONE) :
			type(type) {

	}

	virtual const char* entry_type();

	virtual ~EquipmentEntry() {
	}

	CoreStats& core_stat_modifier() {
		return stat_modifiers.core_mod;
	}
	DamageStats& damage_modifier() {
		return stat_modifiers.damage_mod;
	}
	ArmourStats& armour_modifier() {
		return stat_modifiers.armour_mod;
	}

	CoreStatMultiplier& resistance() {
		return armour_modifier().resistance;
	}
	CoreStatMultiplier& damage_reduction() {
		return armour_modifier().damage_reduction;
	}
	CoreStatMultiplier& magic_resistance() {
		return armour_modifier().magic_resistance;
	}
	CoreStatMultiplier& magic_reduction() {
		return armour_modifier().magic_reduction;
	}

	int number_of_equip_slots() {
		if (type == RING) {
			return 2;
		}
		return 1;
	}

	equip_type type;
	// Stat modifiers while wearing this equipment
	StatModifiers stat_modifiers;
	CooldownModifiers cooldown_modifiers;
	// Status effects from wearing this armour
	StatusEffectModifiers effect_modifiers;
};

equipment_id get_equipment_by_name(const char* name);
EquipmentEntry& get_equipment_entry(projectile_id id);

#endif /* EQUIPMENTENTRY_H_ */
