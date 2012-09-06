/*
 * items.h:
 *  Define item state. These are defined in terms of a base item, applied properties, and a quantity.
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

// Defined in terms of a base item, applied properties, and a quantity.
struct Item {
	item_id id;
	ItemProperties properties;
	int amount;

	ItemEntry& item_entry() const;
	EquipmentEntry& equipment_entry() const;
	ProjectileEntry& projectile_entry() const;
	WeaponEntry& weapon_entry() const;

	bool is_normal_item() const;
	bool is_equipment() const;
	bool is_projectile() const;
	bool is_weapon() const;
	bool empty() const {
		return amount == 0 && id == NO_ITEM;
	}

	Item with_amount(int new_amount) const {
		LANARTS_ASSERT(this->id != NO_ITEM);
		return Item(id, new_amount, properties);
	}

	void add_copies(int amount) {
		LANARTS_ASSERT(this->id != NO_ITEM);
		this->amount += amount;
	}

	void remove_copies(int amount) {
		LANARTS_ASSERT(this->id != NO_ITEM);
		LANARTS_ASSERT(this->amount >= amount);
		this->amount -= amount;
		if (this->amount == 0) {
			clear();
		}
	}
	bool operator==(const Item& item) const;

	void clear() {
		id = NO_ITEM;
		properties = ItemProperties();
	}

	explicit Item(item_id id = NO_ITEM, int amount = 1,
			ItemProperties properties = ItemProperties()) :
			id(id), properties(properties), amount(amount) {
		if (id == NO_ITEM) {
			this->amount = 0;
		}
	}

	bool is_same_item(const Item& item) const;

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
};


typedef Item Weapon;
typedef Item Projectile;
typedef Item Equipment;

#endif /* ITEMS_H_ */
