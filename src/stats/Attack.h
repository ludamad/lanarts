/*
 * Attack.h:
 *  Represents all the data required for an attack. Common to projectiles & melee weapons.
 */

#ifndef ATTACK_H_
#define ATTACK_H_

#include "stat_modifiers.h"
#include "LuaAction.h"

class Attack {
public:
	DamageModifiers damage;

	int range, cooldown;
	sprite_id attack_sprite;
	LuaAction attack_action;

	Attack() :
			range(0), cooldown(0), attack_sprite(-1) {
	}

	void init(lua_State* L) {
		attack_action.init(L);
	}
};

#endif /* ATTACK_H_ */
