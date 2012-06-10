/*
 * items.h:
 *  Define item states. These are defined in terms of a base item, and applied bonuses.
 */

#ifndef ITEMS_H_
#define ITEMS_H_

#include "../util/game_basic_structs.h"

#include "stats.h"

struct Item;
struct ItemEntry;
struct ProjectileEntry;
struct WeaponEntry;

struct Bonuses {
	DerivedStats magic, physical;

	bool operator==(const Bonuses& bonuses) const;
};

struct Armour {
	armour_id id;
	Bonuses bonuses;
	Armour(armour_id id, Bonuses bonuses = Bonuses()) :
			id(id), bonuses(bonuses) {
	}
};

struct Projectile {
	projectile_id id;
	Bonuses bonuses;

	Item as_item() const;
	ItemEntry& item_entry() const;
	ProjectileEntry& projectile_entry() const;
	Projectile(projectile_id id = -1, Bonuses bonuses = Bonuses()) :
			id(id), bonuses(bonuses) {
	}
	bool valid_projectile() const {
		return id >= 0;
	}

	bool operator==(const Projectile& projectile) const;
};

struct Weapon {
	weapon_id id;
	Bonuses bonuses;

	Item as_item() const;
	ItemEntry& item_entry() const;
	WeaponEntry& weapon_entry() const;
	Weapon(weapon_id id = 0, Bonuses bonuses = Bonuses()) :
			id(id), bonuses(bonuses) {
	}
	bool operator==(const Weapon& weapon) const;
};

struct Item {
	item_id id;
	bool is_artifact;
	Bonuses bonuses;

	ItemEntry& item_entry() const;
	Armour as_armour() const;
	Projectile as_projectile() const;
	Weapon as_weapon() const;

//	ArmourEntry& armour_entry();
	ProjectileEntry& projectile_entry() const;
	WeaponEntry& weapon_entry() const;

	bool is_normal_item() const;
	bool is_armour() const;
	bool is_projectile() const;
	bool is_weapon() const;

	Item(item_id id = -1, Bonuses bonuses = Bonuses()) :
			id(id), bonuses(bonuses) {
	}

	bool operator==(const Item& item) const;
};
#endif /* ITEMS_H_ */
