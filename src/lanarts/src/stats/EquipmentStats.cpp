/*
 * EquipmentStats.cpp
 *  Represents all the possessions and equipped items of a player
 */

#include <lcommon/SerializeBuffer.h>

#include "items/ItemEntry.h"
#include "items/ProjectileEntry.h"
#include "items/WeaponEntry.h"

#include "items/items.h"

#include "EquipmentStats.h"

void EquipmentStats::deequip_type(int equipment_type) {
	inventory.deequip_type(equipment_type);
}
bool EquipmentStats::valid_to_use(const Item& item) {
	if (item.is_normal_item()) {
		return true;
	}
	switch (item.equipment_entry().type) {
	case EquipmentEntry::AMMO:
		return projectile_valid_to_equip(inventory, item);
	}
	return true;
}
void EquipmentStats::equip(itemslot_t slot) {
	inventory.equip(slot);
}

bool EquipmentStats::use_ammo(int amnt) {
	LANARTS_ASSERT(has_projectile());
	return projectile_slot().remove_copies(amnt);
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
	return inventory.get_equipped(EquipmentEntry::AMMO) != -1;
}

ItemSlot& EquipmentStats::weapon_slot() {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::WEAPON);
	return inventory.get(slot);
}

ItemSlot& EquipmentStats::projectile_slot() {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::AMMO);
	return inventory.get(slot);
}

ItemSlot& EquipmentStats::armour_slot() {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::BODY_ARMOUR);
	return inventory.get(slot);
}

Weapon EquipmentStats::weapon() const {
	return equipped_weapon(inventory);
}

Projectile EquipmentStats::projectile() const {
	return equipped_projectile(inventory);
}

Equipment EquipmentStats::armour() const {
	return equipped_armour(inventory);
}

