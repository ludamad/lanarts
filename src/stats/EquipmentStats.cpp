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
	if (pentry.is_standalone())
		return true;
	if (pentry.weapon_class == weapon().weapon_entry().weapon_class)
		return true;
	return false;
}

void EquipmentStats::deequip_type(int equipment_type) {
	inventory.deequip_type(equipment_type);
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
void EquipmentStats::equip(itemslot_t slot) {
	inventory.equip(slot);
}

void EquipmentStats::use_ammo(int amnt) {
	LANARTS_ASSERT(has_projectile());
	projectile_slot().remove_copies(amnt);
}

void EquipmentStats::serialize(SerializeBuffer& serializer) {
	inventory.serialize(serializer);
//	_weapon.serialize(serializer);
//	_projectile.serialize(serializer);
//	_armour.serialize(serializer);
	serializer.write_int(money);
}

void EquipmentStats::deserialize(SerializeBuffer& serializer) {
	inventory.deserialize(serializer);
//	_weapon.deserialize(serializer);
//	_projectile.deserialize(serializer);
//	_armour.deserialize(serializer);
	serializer.read_int(money);
}

bool EquipmentStats::has_weapon() {
	return inventory.get_equipped(EquipmentEntry::WEAPON) != -1;
}

bool EquipmentStats::has_armour() {
	return inventory.get_equipped(EquipmentEntry::BODY_ARMOUR) != -1;
}

bool EquipmentStats::has_projectile() {
	return inventory.get_equipped(EquipmentEntry::PROJECTILE) != -1;
}

ItemSlot& EquipmentStats::weapon_slot() {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::WEAPON);
	return inventory.get(slot);
}

ItemSlot& EquipmentStats::projectile_slot() {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::PROJECTILE);
	return inventory.get(slot);
}

ItemSlot& EquipmentStats::armour_slot() {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::BODY_ARMOUR);
	return inventory.get(slot);
}

Weapon EquipmentStats::weapon() const {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::WEAPON);
	if (slot != -1) {
		return inventory.get(slot).item;
	} else {
		return Weapon();
	}
}

Projectile EquipmentStats::projectile() const {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::PROJECTILE);
	if (slot != -1) {
		return inventory.get(slot).item;
	} else {
		return Projectile();
	}
}

Equipment EquipmentStats::armour() const {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::BODY_ARMOUR);
	if (slot != -1) {
		return inventory.get(slot).item;
	} else {
		return Equipment();
	}
}

