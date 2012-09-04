/*
 * items.h:
 *  Define item states. These are defined in terms of a base item, and applied properties.
 */

#ifndef ITEMS_H_
#define ITEMS_H_

#include "../lanarts_defines.h"

#include "stats.h"

struct _Item;
struct _ArmourEntry;
struct _ItemEntry;
struct _ProjectileEntry;
class WeaponEntry;

class SerializeBuffer;

enum item_flags {
	NOFLAGS = 0, CURSED = 1, GOLDEN = 2
};

struct _ItemProperties {
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

	_ItemProperties() :
			flags(NOFLAGS), unknownness(1) {
	}

	bool operator==(const _ItemProperties& properties) const;

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

struct _Armour {
	armour_id id;
	_ItemProperties properties;

	_Item as_item() const;
	_ItemEntry& item_entry() const;

	_Armour(armour_id id, _ItemProperties properties = _ItemProperties()) :
			id(id), properties(properties) {
	}
	_ArmourEntry& armour_entry() const;

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

struct _Projectile {
	projectile_id id;
	_ItemProperties properties;

	_Item as_item() const;
	_ItemEntry& item_entry() const;
	_ProjectileEntry& projectile_entry() const;
	_Projectile(projectile_id id = -1, _ItemProperties properties =
			_ItemProperties()) :
			id(id), properties(properties) {
	}
	bool valid_projectile() const {
		return id >= 0;
	}

	bool operator==(const _Projectile& projectile) const;

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

struct _Weapon {
	weapon_id id;
	_ItemProperties properties;

	_Item as_item() const;
	_ItemEntry& item_entry() const;
	WeaponEntry& weapon_entry() const;
	_Weapon(weapon_id id = -1, _ItemProperties properties = _ItemProperties()) :
			id(id), properties(properties) {
	}
	bool operator==(const _Weapon& weapon) const;

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

struct _Item {
	item_id id;
	_ItemProperties properties;

	_ItemEntry& item_entry() const;
	_Armour as_armour() const;
	_Projectile as_projectile() const;
	_Weapon as_weapon() const;

//	ArmourEntry& armour_entry();
	_ProjectileEntry& projectile_entry() const;
	WeaponEntry& weapon_entry() const;

	bool is_normal_item() const;
	bool is_armour() const;
	bool is_projectile() const;
	bool is_weapon() const;

	_Item(item_id id = -1, _ItemProperties properties = _ItemProperties()) :
			id(id), properties(properties) {
	}

	bool operator==(const _Item& item) const;

	void serialize(SerializeBuffer& serialize);
	void deserialize(SerializeBuffer& serialize);
};

#endif /* ITEMS_H_ */
