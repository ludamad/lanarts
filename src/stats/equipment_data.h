/*
 * equipment_data.h
 *
 *  Created on: Aug 7, 2012
 *      Author: adomurad
 */

#ifndef EQUIPMENT_DATA_H_
#define EQUIPMENT_DATA_H_

//struct ProjectileEntry {
//	std::string name, description;
//	std::string weapon_class; //Compatible with this weapon class
//
//	// Cost when appearing in shops (if an item), if (0,0) will not appear in shops.
//	Range shop_cost;
//
//	sprite_id item_sprite, attack_sprite;
//
//	CoreStatMultiplier power, damage;
//	float percentage_magic; //Conversely the rest is percentage physical
//	float resist_modifier; // How much resistance can resist this attack, lower for fast attacks
//
//	int drop_chance; //out of 100
//	//If unarmed projectile, range/cooldown used
//	//or if larger than base weapon's
//	float speed;
//	bool can_wall_bounce;
//	int number_of_target_bounces;
//	int cooldown, range, radius;
//
//	LuaValue on_hit_func;

//
//struct WeaponEntry {
//	std::string name, description;
//	std::string weapon_class;
//
//	// Cost when appearing in shops (if an item), if (0,0) will not appear in shops.
//	Range shop_cost;
//
//	bool uses_projectile;
//	int max_targets;
//	CoreStatMultiplier power, damage;
//	float percentage_magic; //Conversely the rest is percentage physical
//	float resist_modifier; // How much resistance can resist this attack, lower for fast attacks
//
//	int range, dmgradius, cooldown;
//	sprite_id item_sprite, attack_sprite;
//	projectile_id created_projectile; // for infinite ammo weapons
//	LuaValue on_hit_func;
//
//	WeaponEntry() :
//			uses_projectile(false), max_targets(0), percentage_magic(0.0f), resist_modifier(
//					1.0f), range(0), dmgradius(0), cooldown(0), item_sprite(-1), attack_sprite(
//					-1), created_projectile(-1) {
//	}
//
//	void init(lua_State* L) {
//		on_hit_func.initialize(L);
//	}
//};

#include "../lanarts_defines.h"
#include <vector>

struct DamageModifiers {
	CoreStatMultiplier power, damage;
	float magic_percentage, physical_percentage;
	float resistability;
	DamageModifiers() :
			magic_percentage(0.0f), physical_percentage(0.0f), resistability(
					1.0f) {
	}
};

struct DefenceModifiers {
	CoreStatMultiplier resistance, damage_reduction;
	CoreStatMultiplier magic_resistance, magic_reduction;
};

//Stat modifiers that trivially stack
struct StatModifiers {
	// additive
	CoreStats core_mod;
	DamageModifiers damage_mod;
	DefenceModifiers defence_mod;
	// multiplicative
	float spell_cooldown_mult, melee_cooldown_mult;
	StatModifiers() :
			spell_cooldown_mult(0.0f), melee_cooldown_mult(0.0f) {
	}
};

struct EffectModifiers {
	std::vector<effect_id> permanent_effects;
};

//For weapons and projectiles only
struct AttackStats {
	// For weapons and standalone projectiles only
	int cooldown;
	// additive
	int projectile_speed;
};

struct EquipmentEntry {
	enum equipslot_t {
		UNEQUIPPED, WEAPON, PROJECTILE, BODY_ARMOUR, HELMET, RING
	};
	std::string name, description;
	equipslot_t type;

	// Cost when appearing in shops (if an item), if (0,0) will not appear in shops.
	Range shop_cost;

	sprite_id item_sprite, usage_sprite;

	AttackStats attack_stats;
	StatModifiers stat_mod;

	bool is_nothing() {
		return type == UNEQUIPPED;
	}
	bool is_weapon() {
		return type == UNEQUIPPED || type == WEAPON;
	}
	bool is_projectile() {
		return type == UNEQUIPPED || type == PROJECTILE;
	}
	bool is_body_armour() {
		return type == UNEQUIPPED || type == BODY_ARMOUR;
	}
	bool is_helmet() {
		return type == UNEQUIPPED || type == HELMET;
	}
	bool is_ring() {
		return type == UNEQUIPPED || type == RING;
	}

	EquipmentEntry() :
			type(UNEQUIPPED), item_sprite(0), usage_sprite(0) {
	}
};

typedef EquipmentEntry ProjectileEntry;
typedef EquipmentEntry WeaponEntry;

#endif /* EQUIPMENT_DATA_H_ */
