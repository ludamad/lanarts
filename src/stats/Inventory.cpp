/*
 * Inventory.cpp:
 *  Item's held by a player/enemy
 */

#include "../serialize/SerializeBuffer.h"

#include "items/ItemEntry.h"
#include "items/EquipmentEntry.h"
#include "items/ProjectileEntry.h"
#include "items/WeaponEntry.h"

#include "Inventory.h"

bool Inventory::add(const Item& item, bool equip_as_well) {
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

	if (equip_as_well) {
		equip(slot);
	}

	return false;
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
	itemslot_t slot = get_equipped(EquipmentEntry::PROJECTILE);
	if (slot != -1) {
		ItemSlot& itemslot = get(slot);
		if (!valid_to_use_projectile(*this, itemslot.item)) {
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

bool valid_to_use_projectile(const Inventory& inventory,
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
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::PROJECTILE);
	if (slot != -1) {
		return inventory.get(slot).item;
	} else {
		return Projectile();
	}
}

Equipment equipped_armour(const Inventory & inventory) {
	itemslot_t slot = inventory.get_equipped(EquipmentEntry::BODY_ARMOUR);
	if (slot != -1) {
		return inventory.get(slot).item;
	} else {
		return Equipment();
	}
}

