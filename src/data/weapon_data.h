/*
 * weapon_data.h
 *  Represents projectile & weapon type data loaded from the yaml
 */

#ifndef WEAPON_DATA_H_
#define WEAPON_DATA_H_
#include <cstdlib>
#include <cstring>
#include <vector>
#include "../world/objects/GameInst.h"

struct StatModifier {
	float strength_mult, magic_mult, defence_mult;
	StatModifier() {
		memset(this, 0, sizeof(StatModifier));
	}
};

struct ProjectileEntry {
	std::string name;
	std::string weapon_class; //Compatible with this weapon class

	sprite_id item_sprite, attack_sprite;
	Range damage_bonus;
	int break_chance;//out of 100
};

struct WeaponEntry {
	std::string name;
	std::string weapon_class;
	bool projectile;
	int max_targets;
	Range base_damage;
	StatModifier damage_multiplier;
	int range, dmgradius, cooldown;
	sprite_id item_sprite, attack_sprite;
	projectile_id created_projectile; // for infinite ammo weapons

	WeaponEntry(const std::string& name, const std::string& weapon_class,
			bool projectile, int max_targets, const Range& base_damage,
			const StatModifier& dmgm, int range, int cooldown, int dmgradius,
			sprite_id item_spr, sprite_id attack_spr,
			projectile_id created_projectile = -1) :
			name(name), weapon_class(weapon_class), projectile(projectile), max_targets(
					max_targets), base_damage(base_damage), damage_multiplier(
					dmgm), range(range), dmgradius(dmgradius), cooldown(
					cooldown), item_sprite(item_spr), attack_sprite(attack_spr), created_projectile(
					created_projectile) {
	}
};

extern std::vector<ProjectileEntry> game_projectile_data;
extern std::vector<WeaponEntry> game_weapon_data;

#endif /* WEAPON_DATA_H_ */
