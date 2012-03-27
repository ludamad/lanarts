/*
 * weapon_data.h
 *
 *  Created on: March 26, 2012
 *      Author: pat goebel
 */

#ifndef WEAPON_DATA_H_
#define WEAPON_DATA_H_
#include <cstdlib>
#include "../world/objects/GameInst.h"

enum {
  WEAPON_UNARMED = 0,
  WEAPON_DAGGER = 1,
  WEAPON_SHORT_SWORD =  2,
  WEAPON_MACE = 3
};


struct WeaponType { //Currently for melee & ranged
	const char* name;
	bool projectile;
	int base_damage;
	float damage_multiplier;
	int range, cooldown;
	int weapon_sprite;
	int attack_sprite;
	int projectile_speed;

	WeaponType(const char* name, bool projectile , int base_damage, float damage_multiplier, int range, int cooldown,
			int weapon_spr, int attack_spr, int bspeed = 0) :
			name(name), projectile(projectile),base_damage(base_damage), damage_multiplier(damage_multiplier), range(range), cooldown(cooldown),
			weapon_sprite(weapon_spr), attack_sprite(attack_spr), projectile_speed(bspeed) {}
};

extern std::vector<WeaponType> game_weapon_data;

#endif /* WEAPON_DATA_H_ */
