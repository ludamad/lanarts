/*
 * EquipmentEntry.h:
 *  Represents an equippable item.
 */

#ifndef EQUIPMENTENTRY_H_
#define EQUIPMENTENTRY_H_

#include "lanarts_defines.h"

#include <luawrap/LuaValue.h>
#include "../stat_modifiers.h"

#include "ItemEntry.h"

class EquipmentEntry: public ItemEntry {
public:
	enum equip_type {
		NONE, WEAPON, AMMO, BODY_ARMOUR, RING, BOOTS, GLOVES, HEADGEAR, AMULET, BELT, LEGWEAR, LANART
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
	CoreStatMultiplier& magic_resistance() {
		return armour_modifier().magic_resistance;
	}

	int number_of_equip_slots() {
		if (type == RING) {
			return 2;
		}
		return 1;
	}

	virtual void parse_lua_table(const LuaValue& table);

	equip_type type;
	// Stat modifiers while wearing this equipment
	StatModifiers stat_modifiers;
	CooldownModifiers cooldown_modifiers;
	// Status effects from wearing this armour
	StatusEffectModifiers effect_modifiers;
	SpellsKnown spells_granted;
};

equipment_id get_equipment_by_name(const char* name);
EquipmentEntry& get_equipment_entry(projectile_id id);

#endif /* EQUIPMENTENTRY_H_ */
