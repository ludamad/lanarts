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
	bool valid_to_use_projectile(const Item& proj);
	bool valid_to_use(const Item& item);
	void equip(const Item& item);
	void deequip_projectiles();
	void deequip_weapon();
	void deequip_armour();
	void deequip(int equipment_type);

	void use_ammo(int amnt = 1);

	bool has_weapon() {
		return _weapon.id > NO_ITEM;
	}
	bool has_armour() {
		return _armour.id > NO_ITEM;
	}
	bool has_projectile() {
		return _projectile.id > NO_ITEM;
	}

	ItemSlot& get_item(itemslot_t i) {
		return inventory.get(i);
	}

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

	Weapon& weapon() {
		return _weapon;
	}
	Projectile& projectile() {
		return _projectile;
	}
	Equipment& armour() {
		return _armour;
	}

	Inventory inventory;
	money_t money;
private:
	Weapon _weapon;
	Projectile _projectile;
	Equipment _armour;
};

#endif /* EQUIPMENTSTATS_H_ */
