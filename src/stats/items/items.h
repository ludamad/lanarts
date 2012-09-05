/*
 * items.h:
 *  Define item state. These are defined in terms of a base item, and applied properties.
 */

#ifndef ITEMS_H_
#define ITEMS_H_

#include "../stat_modifiers.h"

class ItemEntry;
class EquipmentEntry;
class ProjectileEntry;
class WeaponEntry;

class SerializeBuffer;

enum item_flags {
	NOFLAGS = 0, CURSED = 1, GOLDEN = 2
};

struct ItemProperties {
	//// For projectiles/weapons:
	DamageStats damage;

	//// For equipment items:
	// Stat modifiers while wearing this equipment
	StatModifiers stat_modifiers;
	CooldownModifiers cooldown_modifiers;
	// Status effects from wearing this armour
	StatusEffectModifiers effect_modifiers;

	item_flags flags;

	// Depending on your skill, using an item will lower this property until it is known
	// XXX: Have a different stat for different players ? (Might be important for PvP)
	int unknownness;

	bool is_identified() {
		return unknownness <= 0;
	}
	void identify() {
		unknownness = 0;
	}

	ItemProperties() :
			flags(NOFLAGS), unknownness(1) {
	}

	bool operator==(const ItemProperties& properties) const;

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
};

struct Item {
	item_id id;
	ItemProperties properties;

	ItemEntry& item_entry() const;
	EquipmentEntry& equipment_entry() const;
	ProjectileEntry& projectile_entry() const;
	WeaponEntry& weapon_entry() const;

	bool is_normal_item() const;
	bool is_equipment() const;
	bool is_projectile() const;
	bool is_weapon() const;

	Item(item_id id = -1, ItemProperties properties = ItemProperties()) :
			id(id), properties(properties) {
	}

	bool operator==(const Item& item) const;

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
};


#endif /* ITEMS_H_ */
