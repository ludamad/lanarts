/*
 * Equipment.h:
 *  Represents all the possessions and equipped items of a player
 */

#ifndef EQUIPMENT_H_
#define EQUIPMENT_H_

#include "../lanarts_defines.h"

#include "Inventory.h"

#include "items.h"

class Equipment {
public:
	Equipment() :
			weapon(0), projectile(-1), projectile_amnt(0), armour(0), money(0) {
	}
	bool valid_to_use_projectile(const _Projectile& proj);
	bool valid_to_use(const _Item& item);
	void equip(const _Item& item, int amnt = 1);
	void deequip_projectiles();
	void deequip_weapon();
	void deequip_armour();
	void deequip(int equipment_type);

	void use_ammo(int amnt = 1);

	bool has_weapon(){ return weapon.id > 0; }
	bool has_armour(){ return armour.id > 0; }
	bool has_projectile(){ return projectile.id > -1; }


	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

	Inventory inventory;
	_Weapon weapon;
	_Projectile projectile;
	int projectile_amnt;
	Armour armour;
	money_t money;
};


#endif /* EQUIPMENT_H_ */
