/*
 * Inventory.cpp:
 *  Item's held by a player/enemy
 */

#include <lcommon/SerializeBuffer.h>
#include <algorithm>

#include "items/ItemEntry.h"
#include "items/EquipmentEntry.h"
#include "items/ProjectileEntry.h"
#include "items/WeaponEntry.h"

#include "Inventory.h"

template <typename Func>
static Range find_slice_forgive1(Inventory& inv, int start_index, const Func& f_matches) {
	// Use a greedy algorithm to find a string of indices where 'f_matches' is *mostly* true.
	// Precisely: We increment an index as long as one of our next 2 members has 'f_matches' true,
	// and return a range up until that index.
	auto& slots = inv.raw_slots();
	for (int i = start_index; i < inv.max_size(); i++) {
		bool matches = f_matches(slots[i]);
		// If we
		if (!matches && i + 1 < inv.max_size()) {
			matches = f_matches(slots[i+1]);
		}
		if (!matches) {
			return Range {start_index, i};
		}
	}
	return Range {start_index, inv.max_size()};
}

static const std::string ONE_TIME = "One-time Use";
static const std::string EVOCABLE = "Evocable";
static const std::string KEY = "Key";

void Inventory::sort() {
	auto calc_rank = [](const ItemSlot& slot) -> int {
		if (slot.empty()) {
			// Make sure empty slots are lost
			return 100;
		}
		int rank = 0;
		if (slot.item_entry().entry_type() == ONE_TIME) {
			return rank;
		}
		rank++;
		if (slot.item.is_weapon()) {
			return rank;
		}
		rank++;
		if (slot.item_entry().entry_type() == EVOCABLE) {
			return rank;
		}
		rank++;
		if (slot.item.is_equipment()) {
			return rank;
		}
		rank++;
		if (slot.item_entry().entry_type() == KEY) {
			return rank;
		}
		rank++;
		return rank;
	};

	std::stable_sort(raw_slots().begin(), raw_slots().end(), [&](const ItemSlot& a, const ItemSlot& b) {
		return calc_rank(a) < calc_rank(b);
	});
}

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

	if (slot == -1) {
	    return -1;
	}

	if (equip_as_well || item.is_equipment()) {
		equip(slot, /*force equip?*/ equip_as_well);
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

void Inventory::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write_int(items.size());
	for (int i = 0; i < items.size(); i++) {
		items[i].item.serialize(gs, serializer);
		serializer.write_byte(items[i].equipped);
	}
}

void Inventory::deserialize(GameState* gs, SerializeBuffer& serializer) {
	int size;
	serializer.read_int(size);
	items.resize(size);
	for (int i = 0; i < items.size(); i++) {
		items[i].item.deserialize(gs, serializer);
		items[i].equipped = serializer.read_byte();
	}
}

static EquipmentEntry::equip_type SPECIAL_SLOTS[] = {EquipmentEntry::RING, EquipmentEntry::AMULET, EquipmentEntry::BODY_ARMOUR};


static bool have_equipped(const Inventory& inv, const std::string& name) {
    for (auto& slot : inv.raw_slots()) {
        if (!slot.is_equipped()) {
            continue;
        }
        if (slot.item.equipment_entry().name == name) {
            return true;
        }
    }
    return false;
}

static int number_of_equip_slots(const Inventory& inv, int type) {
    int max_slots = 1;
    if (type == EquipmentEntry::RING) {
        max_slots += 1;
        if (have_equipped(inv, "Ring of Ethereal Armour")) {
            max_slots = 1;
        } else if (have_equipped(inv, "Amulet of Ringholding")) {
            max_slots += 1;
        }
    } else if (type == EquipmentEntry::AMULET) {
        if (have_equipped(inv, "Amulet of Greed")) {
            max_slots += 2;
        }
    } else if (type == EquipmentEntry::BODY_ARMOUR) {
        if (have_equipped(inv, "Ring of Ethereal Armour")) {
            max_slots += 1;
        }
    }

    return max_slots;
}

void Inventory::__dequip_overfilled_slots() {
    // Account for chains of Amulet of Greed's and ring interactions and such:
    while (true) {
        bool did_something = false;
        for (auto type : SPECIAL_SLOTS) {
            int max_slots = number_of_equip_slots(*this, type);
            itemslot_t last_slot = -1;
            int nslots = 0;
            for (;;) {
                    itemslot_t current_slot = get_equipped(type, last_slot);
                    if (current_slot == -1) {
                            break;
                    } else {
                            last_slot = current_slot;
                            if (++nslots > max_slots) {
                                get(last_slot).equipped = false;
                                did_something = true;
                            }
                    }
            }
        }
        if (!did_something) {
            return;
        }
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

void Inventory::equip(itemslot_t i, bool force_equip) {
	ItemSlot& slot = get(i);
	EquipmentEntry& eentry = slot.item.equipment_entry();
	int max_slots = number_of_equip_slots(*this, eentry.type);
        int nslots = 0;

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
	if (nslots >= max_slots) {
	    if (!force_equip) {
            return;
        }
        get(last_slot).equipped = false;
	}
	slot.equipped = true;
	__dequip_overfilled_slots();
	__dequip_projectile_if_invalid();
}
// Returns NULL if nothing equipped
// pass previous result for slots that can have multiple items equipped
void Inventory::deequip_type(int type) {
	itemslot_t slot = get_equipped(type);
	if (slot != -1) {
		get(slot).deequip();
                __dequip_overfilled_slots();
		__dequip_projectile_if_invalid();
	}
}

void Inventory::deequip(itemslot_t i) {
	ItemSlot& slot = get(i);
	slot.deequip();
	__dequip_overfilled_slots();
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

