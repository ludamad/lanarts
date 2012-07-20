/*
 * Equipment.cpp
 *  Represents all the possessions and equipped items of a player
 */

#include "Equipment.h"

#include "../data/item_data.h"
#include "../data/projectile_data.h"
#include "../data/weapon_data.h"

bool Equipment::valid_to_use_projectile(const Projectile& proj) {
	if (!proj.valid_projectile())
		return false;
	ProjectileEntry& pentry = proj.projectile_entry();
	if (pentry.weapon_class == "unarmed")
		return true;
	if (pentry.weapon_class == weapon.weapon_entry().weapon_class)
		return true;
	return false;
}

void Equipment::deequip_projectiles() {
	if (projectile.valid_projectile()) {
		inventory.add(projectile.as_item(), projectile_amnt);
		projectile = -1;
		projectile_amnt = 0;
	}
}

void Equipment::deequip_weapon() {
	if (has_weapon()) {
		inventory.add(weapon.as_item(), 1);
		weapon = Weapon(NONE);

		if (!valid_to_use_projectile(projectile)) {
			deequip_projectiles();
		}
	}
}

void Equipment::deequip(int equipment_type) {
	switch (equipment_type) {
	case ItemEntry::PROJECTILE:
		deequip_projectiles();
		break;
	case ItemEntry::WEAPON:
		deequip_weapon();
		break;
	}
}
bool Equipment::valid_to_use(const Item& item) {
	switch (item.item_entry().equipment_type) {
	case ItemEntry::PROJECTILE:
		return valid_to_use_projectile(item.as_projectile());
	}
	return true;
}
void Equipment::equip(const Item& item, int amnt) {
	switch (item.item_entry().equipment_type) {
	case ItemEntry::WEAPON:
		deequip_projectiles();
		if (has_weapon()) {
			inventory.add(weapon.as_item(), 1);
		}
		weapon = item.as_weapon();
		break;
	case ItemEntry::PROJECTILE:
		if (!(item.as_projectile() == projectile))
			deequip_projectiles();
		projectile = item.as_projectile();
		projectile_amnt += amnt;
		break;
	}
}

void Equipment::use_ammo(int amnt) {
	projectile_amnt -= amnt;
	if (projectile_amnt <= 0) {
		projectile = Projectile(-1);
		projectile_amnt = 0;
	}
}
