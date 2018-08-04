/*
 * Attack.h:
 *  Represents all the data required for an attack. Common to projectiles & melee weapons.
 */

#ifndef ATTACK_H_
#define ATTACK_H_

#include "stat_modifiers.h"
#include "LuaAction.h"

class LuaField;

class Attack {
public:
	CoreStatMultiplier& damage_stats() {
		return damage_modifiers.damage_stats;
	}

	CoreStatMultiplier& power_stats() {
		return damage_modifiers.power_stats;
	}

	float magic_percentage() {
		return damage_modifiers.magic_percentage;
	}
	float physical_percentage() {
		return damage_modifiers.physical_percentage;
	}

	DamageStats damage_modifiers;

	LuaValue alt_action;
	int range = 0, cooldown = 0;
	sprite_id attack_sprite = -1;
	LuaAction attack_action;
	LuaValue on_damage;
};

Attack parse_attack(const LuaField& field);

#endif /* ATTACK_H_ */
