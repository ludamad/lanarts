/*
 * Equipment.h:
 *  Represents all the possessions and equipped items of a player
 */

#ifndef EQUIPMENT_H_
#define EQUIPMENT_H_

#include "../util/game_basic_structs.h"
#include "Inventory.h"

class Equipment {
public:
	Equipment() :
			weapon(0), projectile(-1), projectile_amnt(0), money(0) {
	}
	bool valid_to_use_projectile(projectile_id pid);
	bool valid_to_use(item_id item);
	void equip(item_id item, int amnt = 1);
	void deequip_projectiles();
	void deequip_weapon();
	void deequip(int equipment_type);

	void use_ammo(int amnt = 1);

	bool has_weapon(){ return weapon > 0; }
	bool has_projectile(){ return projectile > -1; }

	Inventory inventory;
	weapon_id weapon;
	projectile_id projectile;
	int projectile_amnt;
	money_t money;
};

#endif /* EQUIPMENT_H_ */
