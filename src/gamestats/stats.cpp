/*
 * stats.cpp
 *
 *  Created on: Mar 24, 2012
 *      Author: 100397561
 */

#include "stats.h"
#include "../util/mtwist.h"
#include "../data/class_data.h"
#include "../data/weapon_data.h"

static int experience_needed_formula(int xplevel) {
	float proportion = pow(xplevel, 1.75);
	return round(proportion) * 50 + 100;
}

Stats::Stats(float speed, int hp, int mp, int strength, int defence, int magic,
		const Attack & melee, const Attack & ranged) :
		classtype(0), movespeed(speed), hp(hp), max_hp(hp), mp(mp), max_mp(mp), hpregen(
				1.0 / 30), mpregen(1.0 / 15), cooldown(0), hurt_cooldown(0), hp_regened(
				0), mp_regened(0), meleeatk(melee), magicatk(ranged), xp(0), xpneeded(
				experience_needed_formula(1)), xplevel(1), strength(strength), defence(
				defence), magic(magic) {
}

void Stats::step() {
	cooldown--;
	if (cooldown < 0)
		cooldown = 0;

	hurt_cooldown--;
	if (hurt_cooldown < 0)
		hurt_cooldown = 0;

	raise_hp(hpregen);
	raise_mp(mpregen);
}

void Stats::raise_hp(float hpgain) {
	hp_regened += hpgain;
	if (hp_regened > 0) {
		hp += floor(hp_regened);
		hp_regened -= floor(hp_regened);
	}
	if (hp > max_hp)
		hp = max_hp;

}

void Stats::raise_mp(float mpgain) {
	mp_regened += mpgain;
	if (mp_regened > 0) {
		mp += floor(mp_regened);
		mp_regened -= floor(mp_regened);
	}
	if (mp > max_mp)
		mp = max_mp;

}

bool Stats::has_cooldown() {
	return cooldown > 0;
}
const int HURT_COOLDOWN = 30;
float Stats::hurt_alpha() {
	if (hurt_cooldown < HURT_COOLDOWN / 2)
		return hurt_cooldown / HURT_COOLDOWN / 2 * 0.7 + 0.3;

	else
		return (HURT_COOLDOWN - hurt_cooldown) / 10 * 0.7 + 0.3;

}

void Stats::set_hurt_cooldown() {
	if (hurt_cooldown == 0)
		hurt_cooldown = HURT_COOLDOWN;

}

void Stats::reset_melee_cooldown() {
	cooldown = meleeatk.cooldown;
}

void Stats::reset_ranged_cooldown() {
	cooldown = magicatk.cooldown;
}

void Stats::reset_melee_cooldown(const Stats & effectivestats) {
	cooldown = effectivestats.meleeatk.cooldown;
}

void Stats::reset_ranged_cooldown(const Stats & effectivestats) {
	cooldown = effectivestats.magicatk.cooldown;
}

bool Stats::hurt(int dmg) {
	hp -= std::max(0, dmg - defence);
	set_hurt_cooldown();
	if (hp < 0) {
		hp = 0;
		return true;
	}
	return false;
}

void Stats::heal_fully() {
	hp = max_hp;
	mp = max_mp;
}

void Stats::gain_level() {
	ClassType& ct = game_class_data[classtype];

	hp += ct.hp_perlevel;
	max_hp += ct.hp_perlevel;

	mp += ct.mp_perlevel;
	max_mp += ct.mp_perlevel;

	defence += ct.def_perlevel;
	strength += ct.str_perlevel;
	magic += ct.mag_perlevel;

	hpregen += ct.hpregen_perlevel;
	mpregen += ct.mpregen_perlevel;

	xplevel++;
}

int Stats::gain_xp(int amnt) {
	int levels_gained = 0;
	xp += amnt;
	while (xp >= xpneeded) {
		gain_level();
		levels_gained++;
		xp -= xpneeded;
		xpneeded = experience_needed_formula(xplevel);
	}
	return levels_gained;
}

int Stats::calculate_statmod_damage(MTwist& mt, StatModifier& sm) {
	float statdmg = strength * sm.strength_mult + defence * sm.defence_mult
			+ magic * sm.magic_mult;
	return round(statdmg);
}

int Stats::calculate_ranged_damage(MTwist& mt, weapon_id weapon_type,
		projectile_id projectile_type) {
	int damage = 0;
	ProjectileEntry& pentry = game_projectile_data[projectile_type];
	if (!pentry.is_unarmed()) {
		damage += calculate_melee_damage(mt, weapon_type);
	}
	StatModifier& sm = pentry.damage_multiplier;
	damage += calculate_statmod_damage(mt, sm) + mt.rand(pentry.damage)
			+ mt.rand(pentry.damage_added);
	return damage;
}
int Stats::calculate_melee_damage(MTwist& mt, weapon_id weapon_type) {
	WeaponEntry& wentry = game_weapon_data[weapon_type];
	int base_damage = mt.rand(wentry.base_damage);
	StatModifier& sm = wentry.damage_multiplier;
	int damage = calculate_statmod_damage(mt, sm) + base_damage;
	return damage;
}
int Stats::calculate_spell_damage(MTwist& mt, int spell_type) {
	int base_damage = mt.rand(4, 8);
	//StatModifier& sm = wtype.damage_multiplier;
	//float statdmg = strength*sm.strength_mult + defence*sm.defence_mult + magic*sm.magic_mult;
	int damage = magic + base_damage;
	return damage * (spell_type + 1);
}

bool CoreStats::hurt(int dmg) {
	hp -= dmg;

	if (hp < 0) {
		hp = 0;
		return true;
	}
	return false;
}

void CoreStats::heal_fully() {
	hp = max_hp;
	mp = max_mp;
}

void CoreStats::heal_hp(float hpgain) {
	hp_regened += hpgain;
	if (hp_regened > 0) {
		hp += floor(hp_regened);
		hp_regened -= floor(hp_regened);
	}
	if (hp > max_hp)
		hp = max_hp;
}

void CoreStats::heal_mp(float mpgain) {
	mp_regened += mpgain;
	if (mp_regened > 0) {
		mp += floor(mp_regened);
		mp_regened -= floor(mp_regened);
	}
	if (mp > max_mp)
		mp = max_mp;
}

float StatMultiplier::calculate(const CoreStats& stats) {
	return stats.strength * strength + stats.defence * defence
			+ stats.magic * magic + stats.willpower * willpower;
}

void CooldownStats::step() {
	if (--action_cooldown < 0)
		action_cooldown = 0;
	if (--rest_cooldown < 0)
		rest_cooldown = 0;
	if (--pickup_cooldown < 0)
		pickup_cooldown = 0;
}

void CooldownStats::reset_action_cooldown(int cooldown) {
	action_cooldown = std::max(cooldown, action_cooldown);
}

void CooldownStats::reset_pickup_cooldown(int cooldown) {
	pickup_cooldown = std::max(cooldown, pickup_cooldown);
}

void CooldownStats::reset_rest_cooldown(int cooldown) {
	rest_cooldown = std::max(cooldown, rest_cooldown);
}

