/*
 * items.h:
 *  Define item states. These are defined in terms of a base item, and applied bonuses.
 */

#ifndef ITEMS_H_
#define ITEMS_H_

#include "../util/game_basic_structs.h"
#include "stats.h"

struct ItemEntry;
struct ProjectilEntry;
struct WeaponEntry;

struct Bonuses {
	DerivedStats magic, physical;
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

	ItemEntry& item_entry();
	ProjectileEntry& projectile_entry();
	Projectile(projectile_id id, Bonuses bonuses = Bonuses()) :
			id(id), bonuses(bonuses) {
	}
};

struct Weapon {
	weapon_id id;
	Bonuses bonuses;

	ItemEntry& item_entry();
	WeaponEntry& weapon_entry();
	Weapon(weapon_id id, Bonuses bonuses = Bonuses()) :
			id(id), bonuses(bonuses) {
	}
};

struct Item {
	item_id id;
	bool is_artifact;
	Bonuses bonuses;

	ItemEntry& item_entry();
	Armour as_armour();
	Projectile as_projectile();
	Weapon as_weapon();

//	ArmourEntry& armour_entry();
	ProjectileEntry& projectile_entry();
	WeaponEntry& weapon_entry();

	bool is_armour();
	bool is_projectile();
	bool is_weapon();

	Item(item_id id, Bonuses bonuses = Bonuses()) :
			id(id), bonuses(bonuses) {
	}
};
#endif /* ITEMS_H_ */
