/*
 * Inventory.cpp:
 *  Item's held by a player/enemy
 */

#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <vector>

#include "lanarts_defines.h"

#include "items/items.h"

class SerializeBuffer;

const int INVENTORY_SIZE = 40;

typedef int itemslot_t;

struct ItemSlot {
	friend class Inventory;

	Item item;
	bool empty() const {
		return item.empty();
	}
	void clear() {
		item.clear();
		equipped = false;
	}
	int amount() const {
		return item.amount;
	}
	ItemEntry& item_entry() const {
		return item.item_entry();
	}
	EquipmentEntry& equipment_entry() const {
		return item.equipment_entry();
	}
	WeaponEntry& weapon_entry() const {
		return item.weapon_entry();
	}
	ProjectileEntry& projectile_entry() const {
		return item.projectile_entry();
	}
	ItemProperties& properties() {
		return item.properties;
	}
	bool is_same_item(const Item& item) const {
		return this->item.is_same_item(item);
	}
	bool remove_copies(int copies) {
		item.remove_copies(copies);
		if (item.amount <= 0) {
			equipped = false;
			return false;
		}
		return true;
	}
	void add_copies(int copies) {
		item.add_copies(copies);
	}
	bool is_equipped() const {
		return equipped;
	}
	int id() const {
		return item.id;
	}
	ItemSlot() :
			equipped(false) {
	}
	bool operator==(const ItemSlot& itemslot) const {
		return item == itemslot.item && equipped == itemslot.equipped;
	}
	void deequip() {
		equipped = false;
	}
private:
	bool equipped;
};

class Inventory {
public:
	Inventory(int size = INVENTORY_SIZE) {
		items.resize(size);
	}
	// returns -1 if full
	itemslot_t add(const Item& item, bool equip_as_well = false);
	ItemSlot& get(itemslot_t i) {
		return items.at(i);
	}

	const ItemSlot& get(itemslot_t i) const {
		return items.at(i);
	}

	bool slot_filled(int i) const {
		return items.at(i).amount() > 0;
	}
	size_t max_size() const {
		return items.size();
	}

	int find_slot(item_id item);
	size_t last_filled_slot() const;

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

	void equip(itemslot_t i, bool force_equip = true);
	void deequip(itemslot_t i);
	void deequip_type(int type);

	// Make this inventory empty
	void clear() {
		items.clear();
	}
	// For performance of iteration
	std::vector<ItemSlot>& raw_slots() {
	    return items;
	}
    // For performance of iteration
    const std::vector<ItemSlot>& raw_slots() const {
        return items;
    }
	itemslot_t get_equipped(int type, itemslot_t last_slot = -1) const;
	void sort();
    weapon_id& get_natural_weapon() {
        return natural_weapon;
    }
    const weapon_id& get_natural_weapon() const {
        return natural_weapon;
    }
private:
	void __dequip_projectile_if_invalid();
	void __dequip_overfilled_slots();
	std::vector<ItemSlot> items;
	weapon_id natural_weapon = NONE;
};

bool projectile_valid_to_equip(const Inventory& inventory, const Projectile& proj);
// returns -1 if no such item
itemslot_t projectile_compatible_weapon(const Inventory& inventory, const Projectile& proj);
// returns false if not possible
bool projectile_smart_equip(Inventory& inventory, itemslot_t itemslot);
bool projectile_smart_equip(Inventory& inventory, const std::string& preferred_class);

// Tries first available melee weapon
// Since we can always go unarmed, does not fail
// TODO: Figure out how curses might play into this
void weapon_smart_equip(Inventory& inventory);

Weapon equipped_weapon(const Inventory& inventory);
Projectile equipped_projectile(const Inventory& inventory);
Equipment equipped_armour(const Inventory& inventory);

#endif // INVENTORY_H
