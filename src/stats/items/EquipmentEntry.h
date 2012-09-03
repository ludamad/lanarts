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
		NONE, WEAPON, PROJECTILE, ARMOUR, RING
	};
	EquipmentEntry(equip_type type = NONE) :
			type(type) {

	}
	virtual ~EquipmentEntry() {
	}
	equip_type type;
	// Stat modifiers while wearing this equipment
	StatModifiers stat_modifiers;
	CooldownModifiers cooldown_modifiers;
	// Status effects from wearing this armour
	StatusEffectModifiers effect_modifiers;
};

#endif /* EQUIPMENTENTRY_H_ */
