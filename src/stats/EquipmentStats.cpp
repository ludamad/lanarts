/*
 * EquipmentStats.cpp
 *  Represents all the possessions and equipped items of a player
 */

#include "../serialize/SerializeBuffer.h"

#include "items/ItemEntry.h"
#include "items/ProjectileEntry.h"
#include "items/WeaponEntry.h"

#include "items/items.h"

#include "EquipmentStats.h"

bool EquipmentStats::valid_to_use_projectile(const Item& proj) {
	if (proj.empty())
		return false;
	ProjectileEntry& pentry = proj.projectile_entry();
	if (pentry.is_unarmed())
		return true;
	if (pentry.weapon_class == _weapon.weapon_entry().weapon_class)
		return true;
	return false;
}

void EquipmentStats::deequip_projectiles() {
	if (!_projectile.empty()) {
		inventory.add(_projectile);
		_projectile.clear();
	}
}

void EquipmentStats::deequip_weapon() {
	if (has_weapon()) {
		inventory.add(_weapon);
		_weapon.clear();

		if (!valid_to_use_projectile(_projectile)) {
			deequip_projectiles();
		}
	}
}

void EquipmentStats::deequip_armour() {
	if (has_armour()) {
		inventory.add(_armour);
		_armour.clear();
	}
}

void EquipmentStats::deequip(int equipment_type) {
	switch (equipment_type) {
	case EquipmentEntry::PROJECTILE:
		deequip_projectiles();
		break;
	case EquipmentEntry::WEAPON:
		deequip_weapon();
		break;
	case EquipmentEntry::ARMOUR:
		deequip_armour();
		break;
	}
}
bool EquipmentStats::valid_to_use(const Item& item) {
	if (item.is_normal_item()) {
		return true;
	}
	switch (item.equipment_entry().type) {
	case EquipmentEntry::PROJECTILE:
		return valid_to_use_projectile(item);
	}
	return true;
}
void EquipmentStats::equip(const Item& item) {
	switch (item.equipment_entry().type) {
	case EquipmentEntry::ARMOUR:
		LANARTS_ASSERT(item.amount == 1);
		if (_armour.properties.flags & CURSED)
			break;
		deequip_armour();
		_armour = item;
		break;
	case EquipmentEntry::WEAPON:
		LANARTS_ASSERT(item.amount == 1);
		if (_weapon.properties.flags & CURSED)
			break;
		deequip_projectiles();
		if (has_weapon()) {
			inventory.add(_weapon);
		}
		_weapon = item;
		break;
	case EquipmentEntry::PROJECTILE:
		if (_projectile.properties.flags & CURSED)
			break;
		if (!(item.is_same_item(_projectile))) {
			deequip_projectiles();
			_projectile = item;
		} else {
			_projectile.add_copies(item.amount);
		}
		break;
	}
}

void EquipmentStats::use_ammo(int amnt) {
	_projectile.remove_copies(amnt);
}

void EquipmentStats::serialize(SerializeBuffer& serializer) {
	inventory.serialize(serializer);
	_weapon.serialize(serializer);
	_projectile.serialize(serializer);
	_armour.serialize(serializer);
	serializer.write_int(money);
}

void EquipmentStats::deserialize(SerializeBuffer& serializer) {
	inventory.deserialize(serializer);
	_weapon.deserialize(serializer);
	_projectile.deserialize(serializer);
	_armour.deserialize(serializer);
	serializer.read_int(money);
}

