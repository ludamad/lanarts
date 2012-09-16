/*
 * EquipmentStats.h:
 *  Represents all the possessions and equipped items of a player
 */

#ifndef EQUIPMENTSTATS_H_
#define EQUIPMENTSTATS_H_

#include "../lanarts_defines.h"

#include "Inventory.h"

#include "items/items.h"

class EquipmentStats {
public:
	EquipmentStats() :
			money(0) {
	}
	bool valid_to_use(const Item& item);

	void equip(itemslot_t slot);
	void deequip_projectiles();
	void deequip_weapon();
	void deequip_armour();
	void deequip_type(int equipment_type);

	void use_ammo(int amnt = 1);

	bool has_weapon();
	bool has_armour();
	bool has_projectile();

	ItemSlot& get_item(itemslot_t i) {
		return inventory.get(i);
	}

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

	ItemSlot& weapon_slot();
	ItemSlot& projectile_slot();
	ItemSlot& armour_slot();

	Weapon weapon() const;
	Projectile projectile() const;
	Equipment armour() const;

	Inventory inventory;
	money_t money;
};

#endif /* EQUIPMENTSTATS_H_ */
