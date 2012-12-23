/*
 * Inventory.cpp:
 *  Item's held by a player/enemy
 */

#ifndef INVENTORY_H
#define INVENTORY_H

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
	void remove_copies(int copies) {
		item.remove_copies(copies);
		if (item.amount == 0) {
			equipped = false;
		}
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
private:
	void deequip() {
		equipped = false;
	}
	bool equipped;
};

class Inventory {
public:
	Inventory(int size = INVENTORY_SIZE) {
		items.resize(size);
	}
	bool add(const Item& item, bool equip_as_well = false);
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

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

	void equip(itemslot_t i);
	void deequip(itemslot_t i);
	void deequip_type(int type);

	itemslot_t get_equipped(int type, itemslot_t last_slot = -1) const;
private:
	void __dequip_projectile_if_invalid();
	std::vector<ItemSlot> items;
};

bool valid_to_use_projectile(const Inventory& inventory, const Projectile& proj);
// returns -1 if no such item
itemslot_t projectile_compatible_weapon(const Inventory& inventory, const Projectile& proj);
// returns false if not possible
bool projectile_smart_equip(Inventory& inventory, itemslot_t itemslot);

Weapon equipped_weapon(const Inventory& inventory);
Projectile equipped_projectile(const Inventory& inventory);
Equipment equipped_armour(const Inventory& inventory);

#endif // INVENTORY_H
