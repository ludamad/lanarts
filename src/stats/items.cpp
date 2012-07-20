/*
 * items.cpp:
 *  Define item states. These are defined in terms of a base item, and applied bonuses.
 */

#include "../data/game_data.h"

#include "items.h"

ItemEntry& Projectile::item_entry() const {
	const std::string& name = projectile_entry().name;
	return game_item_data.at(get_item_by_name(name.c_str()));
}

Item Projectile::as_item() const {
	const std::string& name = projectile_entry().name;
	return Item(get_item_by_name(name.c_str()), bonuses);
}

ProjectileEntry& Projectile::projectile_entry() const {
	return game_projectile_data.at(id);
}

ItemEntry& Weapon::item_entry() const {
	const std::string& name = weapon_entry().name;
	return game_item_data.at(get_item_by_name(name.c_str()));
}

Item Weapon::as_item() const {
	const std::string& name = weapon_entry().name;
	return Item(get_item_by_name(name.c_str()), bonuses);
}

WeaponEntry& Weapon::weapon_entry() const {
	return game_weapon_data.at(id);
}

ItemEntry& Item::item_entry() const {
	LANARTS_ASSERT(id > -1);
	return game_item_data.at(id);
}

Armour Item::as_armour() const {
	return Armour(item_entry().equipment_id, bonuses);
}

ArmourEntry& Armour::armour_entry() const {
	return game_armour_data.at(id);
}

Projectile Item::as_projectile() const {
	return Projectile(item_entry().equipment_id, bonuses);
}

Weapon Item::as_weapon() const {
	return Weapon(item_entry().equipment_id, bonuses);
}

ProjectileEntry& Item::projectile_entry() const {
	LANARTS_ASSERT(is_projectile());
	return game_projectile_data.at(item_entry().equipment_id);
}

WeaponEntry& Item::weapon_entry() const {
	LANARTS_ASSERT(is_weapon());
	return game_weapon_data.at(item_entry().equipment_id);
}

bool Item::is_normal_item() const {
	return item_entry().equipment_type == ItemEntry::NONE;
}

bool Item::is_armour() const {
	return item_entry().equipment_type == ItemEntry::ARMOUR;
}

bool Item::is_projectile() const {
	return item_entry().equipment_type == ItemEntry::PROJECTILE;
}

bool Item::is_weapon() const {
	return item_entry().equipment_type == ItemEntry::WEAPON;
}

bool Bonuses::operator ==(const Bonuses& bonuses) const {
	return magic == bonuses.magic && physical == bonuses.physical;
}

bool Projectile::operator ==(const Projectile& projectile) const {
	return id == projectile.id && bonuses == projectile.bonuses;
}

bool Weapon::operator ==(const Weapon& weapon) const {
	return id == weapon.id && bonuses == weapon.bonuses;
}

bool Item::operator ==(const Item& item) const {
	return id == item.id && bonuses == item.bonuses;
}

