/*
 * Equipment.h:
 *  Represents all the possessions and equipped items of a player
 */

#ifndef EQUIPMENT_H_
#define EQUIPMENT_H_

#include "../util/game_basic_structs.h"

#include "items.h"

#include "Inventory.h"

class Equipment {
public:
	Equipment() :
			weapon(0), projectile(-1), projectile_amnt(0), body_armour(0), money(0) {
	}
	bool valid_to_use_projectile(const Projectile& proj);
	bool valid_to_use(const Item& item);
	void equip(const Item& item, int amnt = 1);
	void deequip_projectiles();
	void deequip_weapon();
	void deequip(int equipment_type);

	void use_ammo(int amnt = 1);

	bool has_weapon(){ return weapon.id > 0; }
	bool has_body_armour(){ return body_armour.id > 0; }
	bool has_projectile(){ return projectile.id > -1; }

	Inventory inventory;
	Weapon weapon;
	Projectile projectile;
	int projectile_amnt;
	Armour body_armour;
	money_t money;
};


#endif /* EQUIPMENT_H_ */
