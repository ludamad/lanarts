/*
 * weapon_data.h
 *  Represents enemy & player weapon (ie, for enemies, natural attacks) data
 *  loaded from the yaml
 */

#ifndef WEAPON_DATA_H_
#define WEAPON_DATA_H_

#include <cstdlib>
#include <string>
#include <vector>

#include "stats.h"

#include "../lanarts_defines.h"

struct WeaponEntry {
	std::string name, description;
	std::string weapon_class;
	bool uses_projectile;
	int max_targets;
	CoreStatMultiplier power, damage;
	float percentage_magic; //Conversely the rest is percentage physical
	float resist_modifier; // How much resistance can resist this attack, lower for fast attacks

	int range, dmgradius, cooldown;
	sprite_id item_sprite, attack_sprite;
	projectile_id created_projectile; // for infinite ammo weapons
	LuaValue on_hit_func;

	WeaponEntry() :
			uses_projectile(false), max_targets(0), percentage_magic(0.0f), resist_modifier(
					1.0f), range(0), dmgradius(0), cooldown(0), item_sprite(-1), attack_sprite(
					-1), created_projectile(-1) {
	}

	void init(lua_State* L) {
		on_hit_func.initialize(L);
	}
};

extern std::vector<WeaponEntry> game_weapon_data;

weapon_id get_weapon_by_name(const char* name);

#endif /* WEAPON_DATA_H_ */
