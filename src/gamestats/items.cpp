/*
 * items.cpp:
 *  Define item states. These are defined in terms of a base item, and applied bonuses.
 */

#include "../data/game_data.h"

#include "items.h"

ItemEntry& Projectile::item_entry() {
	const std::string& name = projectile_entry().name;
	return game_item_data[get_item_by_name(name.c_str())];
}

ProjectileEntry& Projectile::projectile_entry() {
	return game_projectile_data[id];
}

ItemEntry& Weapon::item_entry() {
	const std::string& name = weapon_entry().name;
	return game_item_data[get_item_by_name(name.c_str())];
}

WeaponEntry& Weapon::weapon_entry() {
	return game_weapon_data[id];
}

ItemEntry& Item::item_entry() {
	const std::string& name = weapon_entry().name;
	return game_item_data[get_item_by_name(name.c_str())];
}

Armour Item::as_armour() {
	return Armour(item_entry().equipment_id, bonuses);
}

Projectile Item::as_projectile() {
	return Projectile(item_entry().equipment_id, bonuses);
}

Weapon Item::as_weapon() {
	return Weapon(item_entry().equipment_id, bonuses);
}

ProjectileEntry& Item::projectile_entry() {
	LANARTS_ASSERT(is_projectile());
	return game_projectile_data[item_entry().equipment_id];
}

WeaponEntry& Item::weapon_entry() {
	LANARTS_ASSERT(is_weapon());
	return game_weapon_data[item_entry().equipment_id];
}

bool Item::is_armour() {
	return item_entry().equipment_type == ItemEntry::ARMOUR;
}

bool Item::is_projectile() {
	return item_entry().equipment_type == ItemEntry::PROJECTILE;
}

bool Item::is_weapon() {
	return item_entry().equipment_type == ItemEntry::WEAPON;
}
