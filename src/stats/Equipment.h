/*
 * Equipment.h:
 *  Represents all the possessions and equipped items of a player
 */

#ifndef EQUIPMENT_H_
#define EQUIPMENT_H_

#include "../lanarts_defines.h"

#include "Inventory.h"

#include "items/items.h"
#include "items.h"

class Equipment {
public:
	Equipment() :
			money(0) {
	}
	bool valid_to_use_projectile(const Item& proj);
	bool valid_to_use(const Item& item);
	void equip(const Item& item, int amnt = 1);
	void deequip_projectiles();
	void deequip_weapon();
	void deequip_armour();
	void deequip(int equipment_type);

	void use_ammo(int amnt = 1);

	bool has_weapon() {
		return weapon.id > NO_ITEM;
	}
	bool has_armour() {
		return armour.id > NO_ITEM;
	}
	bool has_projectile() {
		return projectile.id > NO_ITEM;
	}

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

	Inventory inventory;
	Item weapon, projectile, armour;
	money_t money;
};

#endif /* EQUIPMENT_H_ */
