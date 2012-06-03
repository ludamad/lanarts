/*
 * weapon_data.h
 *
 *  Created on: March 26, 2012
 *      Author: pat goebel
 */

#ifndef WEAPON_DATA_H_
#define WEAPON_DATA_H_
#include <cstdlib>
#include <vector>
#include "../world/objects/GameInst.h"

enum {
  WEAPON_UNARMED = 0,
  WEAPON_DAGGER = 1,
  WEAPON_SHORT_SWORD =  2,
  WEAPON_MACE = 3
};


struct StatModifier {
	float strength_mult, magic_mult, defence_mult;
	StatModifier(){
		memset(this, 0, sizeof(StatModifier));
	}
};

struct WeaponEntry { //Currently for melee & ranged
	std::string name;
	bool projectile;
	int max_targets;
	int base_mindmg, base_maxdmg;
	StatModifier damage_multiplier;
	int range, dmgradius, cooldown;
	int weapon_sprite;
	int attack_sprite;
	int projectile_speed;



	WeaponEntry(const std::string& name, bool projectile,
			int max_targets,
			int base_mindamage, int base_maxdamage, const StatModifier& dmgm, int range, int cooldown,
			int dmgradius,
			int weapon_spr, int attack_spr, int bspeed = 0) :
			name(name), projectile(projectile),
			max_targets(max_targets), base_mindmg(base_mindamage),
			base_maxdmg(base_maxdamage), damage_multiplier(dmgm), range(range), dmgradius(dmgradius),
			cooldown(cooldown), weapon_sprite(weapon_spr), attack_sprite(attack_spr), projectile_speed(bspeed) {}
};

extern std::vector<WeaponEntry> game_weapon_data;

#endif /* WEAPON_DATA_H_ */
