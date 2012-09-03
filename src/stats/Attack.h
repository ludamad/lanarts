/*
 * Attack.h:
 *  Represents all the data required for an attack. Common to projectiles & melee weapons.
 */

#ifndef ATTACK_H_
#define ATTACK_H_

class Attack {
public:
	bool uses_projectile;
	int max_targets;
	CoreStatMultiplier power, damage;
	float percentage_magic; //Conversely the rest is percentage physical
	float resist_modifier; // How much resistance can resist this attack, lower for fast attacks

	int range, dmgradius, cooldown;
	sprite_id item_sprite, attack_sprite;
	projectile_id created_projectile; // for infinite ammo weapons
	LuaValue on_hit_func;
};

#endif /* ATTACK_H_ */
