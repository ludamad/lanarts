/*
 * Inventory.cpp:
 *  Item's held by a player/enemy
 */

#include <lcommon/SerializeBuffer.h>

#include "items/ItemEntry.h"
#include "items/EquipmentEntry.h"
#include "items/ProjectileEntry.h"
#include "items/WeaponEntry.h"

#include "Inventory.h"

itemslot_t Inventory::add(const Item& item, bool equip_as_well) {
	itemslot_t slot = -1;
	ItemEntry& ientry = item.item_entry();
	if (ientry.stackable) {
		/* Try to merge with existing entry */
		for (int i = 0; i < items.size(); i++) {
			if (items[i].is_same_item(item) && !items[i].empty()) {
				items[i].item.amount += item.amount;
				slot = i;
				break;
			}
		}
	}
	if (slot == -1) {
		/* Try to add to new slot */
		for (int i = 0; i < items.size(); i++) {
			if (items[i].empty()) {
				items[i].item = item;
				items[i].equipped = false;
				slot = i;
				break;
			}
		}
	}

	if (equip_as_well && slot != -1) {
		equip(slot);
	}

	return slot;
}

int Inventory::find_slot(item_id item) {
	for (int i = 0; i < max_size(); i++) {
		if (get(i).id() == item)
			return i;
	}
	return -1;
}

size_t Inventory::last_filled_slot() const {
	int i = max_size() - 1;
	for (; i >= 0; i--) {
		if (items[i].amount() > 0) {
			return i + 1;
		}
	}

	return 0;
}

void Inventory::serialize(SerializeBuffer& serializer) {
	serializer.write_int(items.size());
	for (int i = 0; i < items.size(); i++) {
		items[i].item.serialize(serializer);
		serializer.write_byte(items[i].equipped);
	}
}

void Inventory::deserialize(SerializeBuffer& serializer) {
	int size;
	serializer.read_int(size);
	items.resize(size);
	for (int i = 0; i < items.size(); i++) {
		items[i].item.deserialize(serializer);
		serializer.read_byte(items[i].equipped);
	}
}

void Inventory::__dequip_projectile_if_invalid() {
	itemslot_t slot = get_equipped(EquipmentEntry::AMMO);
	if (slot != -1) {
		ItemSlot& itemslot = get(slot);
		if (!projectile_valid_to_equip(*this, itemslot.item)) {
			get(slot).deequip();
		}
	}
}

void Inventory::equip(itemslot_t i) {
	ItemSlot& slot = get(i);
	EquipmentEntry& eentry = slot.item.equipment_entry();
	int max_slots = eentry.number_of_equip_slots(), nslots = 0;

	itemslot_t last_slot = -1;
	for (;;) {
		itemslot_t current_slot = get_equipped(eentry.type, last_slot);
		if (current_slot == -1) {
			break;
		} else {
			nslots++;
			last_slot = current_slot;
		}
	}
	// We should never have more equip slots than those available
	LANARTS_ASSERT(nslots <= max_slots);

	if (nslots >= max_slots) {
		get(last_slot).equipped = false;
	}
	slot.equipped = true;
	__dequip_projectile_if_invalid();
}
// Returns NULL if nothing equipped
// pass previous result for slots that can have multiple items equipped
void Inventory::deequip_type(int type) {
	itemslot_t slot = get_equipped(type);
	if (slot != -1) {
		get(slot).deequip();
		__dequip_projectile_if_invalid();
	}
}

void Inventory::deequip(itemslot_t i) {
	ItemSlot& slot = get(i);
	slot.deequip();
	__dequip_projectile_if_invalid();
}

itemslot_t Inventory::get_equipped(int type, itemslot_t last_slot) const {
	int size = max_size();
	for (int i = last_slot + 1; i < size; i++) {
		const ItemSlot& slot = get(i);
		if (!slot.empty() && slot.equipped && slot.item.is_equipment()) {
			EquipmentEntry& eentry = slot.item.equipment_entry();
			if (eentry.type == type) {
				return i;
			}
		}
	}
	return -1;
}

bool projectile_valid_to_equip(const Inventory& inventory,
		const Projectile& proj) {

	if (proj.empty())
		return false;
	ProjectileEntry& pentry = proj.projectile_entry();
	if (pentry.is_standalone())
		return true;
	Weapon weapon = equipped_weapon(inventory);
	if (pentry.weapon_class == weapon.weapon_entry().weapon_class)
		return true;
	return false;
}

Weapon equipped_weapon(const Inventory & inventory) {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::WEAPON);
	if (slot != -1) {
		return inventory.get(slot).item;
	} else {
		return Weapon();
	}
}

Projectile equipped_projectile(const Inventory & inventory) {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::AMMO);
	if (slot != -1) {
		return inventory.get(slot).item;
	} else {
		return Projectile();
	}
}

itemslot_t projectile_compatible_weapon(const Inventory& inventory,
		const Projectile& proj) {
	ProjectileEntry& pentry = proj.projectile_entry();
	int size = inventory.max_size();
	// -- first try option that works with preferred weapon type
	for (itemslot_t i = 0; i < size; i++) {
		const ItemSlot& slot = inventory.get(i);
		if (!slot.empty() && slot.item.is_weapon()) {
			if (slot.weapon_entry().weapon_class == pentry.weapon_class) {
				return i;
			}
		}
	}
	return -1;
}

bool projectile_smart_equip(Inventory& inventory, itemslot_t itemslot) {
	const Projectile& proj = inventory.get(itemslot).item;
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::AMMO);

	if (projectile_valid_to_equip(inventory, proj)) {
		inventory.equip(itemslot);
		return true;
	}
	itemslot_t weapon_slot = projectile_compatible_weapon(inventory, proj);
	if (weapon_slot != -1) {
		inventory.equip(weapon_slot);
		inventory.equip(itemslot);
		return true;
	}
	return false;
}

// Tries to use any available projectiles, returning false if no candidates
bool projectile_smart_equip(Inventory& inventory,
		const std::string& preferred_class) {
	size_t size = inventory.max_size();

	// first try option that works with preferred weapon type
	for (itemslot_t i = 0; i < size; i++) {
		ItemSlot& slot = inventory.get(i);
		if (!slot.empty() && slot.item.is_projectile()
				&& slot.projectile_entry().weapon_class == preferred_class) {
			bool equipped = projectile_smart_equip(inventory, i);
			LANARTS_ASSERT(equipped);
			return true;
		}
	}

	// next try any non-standalone projectile
	for (itemslot_t i = 0; i < size; i++) {
		ItemSlot& slot = inventory.get(i);
		if (!slot.empty() && slot.item.is_projectile()
				&& !slot.projectile_entry().is_standalone()) {
			if (projectile_smart_equip(inventory, i)) {
				return true;
			}
		}
	}

	// next try any standalone projectile
	for (itemslot_t i = 0; i < size; i++) {
		ItemSlot& slot = inventory.get(i);
		if (!slot.empty() && slot.item.is_projectile()) {
			if (projectile_smart_equip(inventory, i)) {
				return true;
			}
		}
	}

	return false;
}

// Tries first available melee weapon
// Since we can always go unarmed, does not fail
// TODO: Figure out how curses might play into this
void weapon_smart_equip(Inventory& inventory) {
	size_t size = inventory.max_size();
	// first try any weapon that does not require a projectile
	for (itemslot_t i = 0; i < size; i++) {
		ItemSlot& slot = inventory.get(i);
		if (!slot.empty() && slot.item.is_weapon()
				&& !slot.weapon_entry().uses_projectile) {
			inventory.equip(i);
			return;
		}
	}
	// Next just go unarmed
	inventory.deequip_type(EquipmentEntry::WEAPON);
	inventory.deequip_type(EquipmentEntry::AMMO);
}

Equipment equipped_armour(const Inventory & inventory) {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::BODY_ARMOUR);
	if (slot != -1) {
		return inventory.get(slot).item;
	} else {
		return Equipment();
	}
}

