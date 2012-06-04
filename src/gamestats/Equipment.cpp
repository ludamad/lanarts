/*
 * Equipment.cpp
 *  Represents all the possessions and equipped items of a player
 */

#include "Equipment.h"

#include "../data/item_data.h"
#include "../data/weapon_data.h"

void Equipment::deequip_projectiles() {
	if (projectile != -1) {
		ProjectileEntry& entry = game_projectile_data[weapon];
		item_id item = get_item_by_name(entry.name.c_str());
		inventory.add(item, projectile_amnt);
		projectile = -1;
		projectile_amnt = 0;
	}
}

void Equipment::equip(item_id item, int amnt) {
	ItemEntry& entry = game_item_data[item];
	switch (entry.equipment_type) {
	case ItemEntry::WEAPON:
		deequip_projectiles();
		if (weapon != 0) {
			WeaponEntry& entry = game_weapon_data[weapon];
			item_id item = get_item_by_name(entry.name.c_str());
			inventory.add(item, 1);
		}
		weapon = game_item_data[item].equipment_id;
		break;
	case ItemEntry::PROJECTILE:
		deequip_projectiles();
		projectile = game_item_data[item].equipment_id;
		projectile_amnt = amnt;
		break;
	}
}

