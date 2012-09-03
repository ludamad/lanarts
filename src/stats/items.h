/*
 * items.h:
 *  Define item states. These are defined in terms of a base item, and applied properties.
 */

#ifndef ITEMS_H_
#define ITEMS_H_

#include "../lanarts_defines.h"

#include "stats.h"

struct Item;
struct ArmourEntry;
struct _ItemEntry;
struct ProjectileEntry;
struct _WeaponEntry;

class SerializeBuffer;

enum item_flags {
	NOFLAGS = 0, CURSED = 1, GOLDEN = 2
};

struct ItemProperties {
	DerivedStats magic, physical;
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

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

struct Armour {
	armour_id id;
	ItemProperties properties;

	Item as_item() const;
	_ItemEntry& item_entry() const;

	Armour(armour_id id, ItemProperties properties = ItemProperties()) :
			id(id), properties(properties) {
	}
	ArmourEntry& armour_entry() const;

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

struct Projectile {
	projectile_id id;
	ItemProperties properties;

	Item as_item() const;
	_ItemEntry& item_entry() const;
	ProjectileEntry& projectile_entry() const;
	Projectile(projectile_id id = -1, ItemProperties properties =
			ItemProperties()) :
			id(id), properties(properties) {
	}
	bool valid_projectile() const {
		return id >= 0;
	}

	bool operator==(const Projectile& projectile) const;

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

struct Weapon {
	weapon_id id;
	ItemProperties properties;

	Item as_item() const;
	_ItemEntry& item_entry() const;
	_WeaponEntry& weapon_entry() const;
	Weapon(weapon_id id = 0, ItemProperties properties = ItemProperties()) :
			id(id), properties(properties) {
	}
	bool operator==(const Weapon& weapon) const;

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

struct Item {
	item_id id;
	ItemProperties properties;

	_ItemEntry& item_entry() const;
	Armour as_armour() const;
	Projectile as_projectile() const;
	Weapon as_weapon() const;

//	ArmourEntry& armour_entry();
	ProjectileEntry& projectile_entry() const;
	_WeaponEntry& weapon_entry() const;

	bool is_normal_item() const;
	bool is_armour() const;
	bool is_projectile() const;
	bool is_weapon() const;

	Item(item_id id = -1, ItemProperties properties = ItemProperties()) :
			id(id), properties(properties) {
	}

	bool operator==(const Item& item) const;

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

#endif /* ITEMS_H_ */
