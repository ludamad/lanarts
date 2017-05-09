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
	Attack() :
			range(0), cooldown(0), attack_sprite(-1) {
	}

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

	void init(lua_State* L) {
		attack_action.init(L);
	}

	DamageStats damage_modifiers;

	int range, cooldown;
	sprite_id attack_sprite;
	LuaAction attack_action;
};

Attack parse_attack(const LuaField& field);

#endif /* ATTACK_H_ */
