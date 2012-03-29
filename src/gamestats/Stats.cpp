/*
 * Stats.cpp
 *
 *  Created on: Mar 24, 2012
 *      Author: 100397561
 */

#include "Stats.h"

Stats::Stats(float speed, int hp, int mp, int strength,
		int dexterity, int magic, const Attack & melee,
		const Attack & ranged) :
		movespeed(speed), hp(hp), max_hp(hp), mp(mp), max_mp(mp), hpregen(
				1.0 / 30), mpregen(1.0 / 15), cooldown(0), hurt_cooldown(0), hp_regened(
				0), mp_regened(0), melee(melee), ranged(ranged), xp(0), xpneeded(
				100), xplevel(1), strength(strength), dexterity(dexterity), magic(magic) {
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
	if (hurt_cooldown < HURT_COOLDOWN/2)
		return hurt_cooldown / HURT_COOLDOWN/2 * 0.7 + 0.3;

	else
		return (HURT_COOLDOWN - hurt_cooldown) / 10 * 0.7 + 0.3;

}

void Stats::set_hurt_cooldown() {
	if (hurt_cooldown == 0)
		hurt_cooldown = HURT_COOLDOWN;

}

void Stats::reset_melee_cooldown() {
	cooldown = melee.cooldown;
}

void Stats::reset_ranged_cooldown() {
	cooldown = ranged.cooldown;
}

void Stats::reset_melee_cooldown(const Stats & effectivestats) {
	cooldown = effectivestats.melee.cooldown;
}

void Stats::reset_ranged_cooldown(const Stats & effectivestats) {
	cooldown = effectivestats.ranged.cooldown;
}

bool Stats::hurt(int dmg) {
	hp -= dmg;
	set_hurt_cooldown();
	if (hp < 0) {
		hp = 0;
		return true;
	}
	return false;
}

void Stats::gain_level() {
	hp += 20;
	max_hp += 20;
	mp += 20;
	max_mp += 20;
	melee.damage += 2;
	ranged.damage += 2;
	xplevel++;
}

void Stats::gain_xp(int amnt) {
	xp += amnt;
	while (xp >= xpneeded) {
		gain_level();
		xp -= xpneeded;
		xpneeded = (xplevel) * 125;
	}
}
