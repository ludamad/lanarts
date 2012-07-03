/*
 * weapon_data.h
 *  Represents projectile & weapon type data loaded from the yaml
 */

#ifndef WEAPON_DATA_H_
#define WEAPON_DATA_H_

#include <cstdlib>
#include <cstring>
#include <vector>

#include "../gamestats/stats.h"

#include "../util/game_basic_structs.h"

#include "../world/objects/GameInst.h"

struct ProjectileEntry {
	std::string name, description;
	std::string weapon_class; //Compatible with this weapon class

	sprite_id item_sprite, attack_sprite;
	CoreStatMultiplier power, damage;
	float percentage_magic;//Conversely the rest is percentage physical

	int drop_chance;//out of 100
	//If unarmed projectile, range/cooldown used
	//or if larger than base weapon's
	float speed;
	int cooldown, range, radius;

	bool is_unarmed(){
		return weapon_class == "unarmed";
	}
};

struct WeaponEntry {
	std::string name, description;
	std::string weapon_class;
	bool uses_projectile;
	int max_targets;
	CoreStatMultiplier power, damage;
	float percentage_magic;//Conversely the rest is percentage physical

	int range, dmgradius, cooldown;
	sprite_id item_sprite, attack_sprite;
	projectile_id created_projectile; // for infinite ammo weapons
};

extern std::vector<ProjectileEntry> game_projectile_data;
extern std::vector<WeaponEntry> game_weapon_data;

projectile_id get_projectile_by_name(const char* name);
weapon_id get_weapon_by_name(const char* name);

#endif /* WEAPON_DATA_H_ */
