/*
 * EquipmentStats.cpp
 *  Represents all the possessions and equipped items of a player
 */

#include <luawrap/luawrap.h>

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

void EquipmentStats::serialize(GameState* gs, SerializeBuffer& serializer) {
	inventory.serialize(gs, serializer);
}

void EquipmentStats::deserialize(GameState* gs, SerializeBuffer& serializer) {
	inventory.deserialize(gs, serializer);
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

static Item parse_as_item(const LuaField& value,
		const char* key = "item") {
	return Item(get_item_by_name(value[key].to_str()),
			luawrap::defaulted(value, "amount", 1));
}

static Inventory parse_inventory(const LuaField& value) {
	Inventory ret;
	int len = value.objlen();
	for (int i = 1; i <= len; i++) {
		ret.add(parse_as_item(value[i]));
	}
	return ret;
}

EquipmentStats parse_equipment(const LuaField& value) {
	EquipmentStats ret;
	if (value.has("inventory")) {
		ret.inventory = parse_inventory(value["inventory"]);
	}

	if (value.has("weapon")) {
		Item item = parse_as_item(value, "weapon");
		ret.inventory.add(item, true);
	}

	if (value.has("projectile")) {
		Item item = parse_as_item(value["projectile"]);
		ret.inventory.add(item, true);
	}
	return ret;
}
