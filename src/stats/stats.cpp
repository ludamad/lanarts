/**
 * stats.h:
 *  Represents the various kinds of attacks.
 *  Note that EffectiveStats is first created with all static information, ie
 *  without attack choice info, and then attack information is used to complete it.
 */

#include "../util/mtwist.h"

#include "items/WeaponEntry.h"

#include "class_data.h"

#include "combat_stats.h"

#include "stats.h"

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

void CoreStats::heal_hp(float hpgain, int maxhp) {
	hp_regened += hpgain;
	if (hp_regened > 0) {
		hp += floor(hp_regened);
		hp_regened -= floor(hp_regened);
	}
	if (hp > maxhp)
		hp = maxhp;
}

void CoreStats::step(const CoreStats& effective_stats, float hp_regen_mod, float mp_regen_mod) {
	heal_hp(hpregen * hp_regen_mod, effective_stats.max_hp);
	heal_mp(mpregen * mp_regen_mod, effective_stats.max_mp);
}

void CoreStats::heal_mp(float mpgain, int maxmp) {
	mp_regened += mpgain;
	if (mp_regened > 0) {
		mp += floor(mp_regened);
		mp_regened -= floor(mp_regened);
	}

	if (mp > maxmp)
		mp = maxmp;
}

void CoreStats::apply_as_bonus(const CoreStats& bonus_stats) {
//	hp += bonus_stats.hp;
	max_hp += bonus_stats.max_hp;

//	mp += bonus_stats.mp;
	max_mp += bonus_stats.max_mp;

	strength += bonus_stats.strength;
	defence += bonus_stats.defence;
	magic += bonus_stats.magic;
	willpower += bonus_stats.willpower;

	hpregen += bonus_stats.hpregen;
	mpregen += bonus_stats.mpregen;
}

Range CoreStatMultiplier::calculate_range(const CoreStats& stats) const {
	float stats_sum = stats.strength * strength + stats.defence * defence
			+ stats.magic * magic + stats.willpower * willpower;
	return Range(base.min + round(stats_sum), base.max + round(stats_sum));
}

float CoreStatMultiplier::calculate(MTwist& mt, const CoreStats& stats) const {
	float stats_sum = stats.strength * strength + stats.defence * defence
			+ stats.magic * magic + stats.willpower * willpower;
	return stats_sum + mt.rand(base);
}

EffectiveAttackStats EffectiveStats::with_attack(MTwist& mt,
		const AttackStats& attack) const {
	EffectiveAttackStats ret;
	ret.cooldown = attack.atk_cooldown() * cooldown_mult;
	ret.damage = attack.atk_damage(mt, *this);
	ret.power = attack.atk_power(mt, *this);
	ret.magic_percentage = attack.atk_percentage_magic();
	ret.resist_modifier = attack.atk_resist_modifier();
	return ret;
}

void CooldownStats::step() {
	if (--action_cooldown < 0)
		action_cooldown = 0;
	if (--rest_cooldown < 0)
		rest_cooldown = 0;
	if (--pickup_cooldown < 0)
		pickup_cooldown = 0;
	if (--hurt_cooldown < 0)
		hurt_cooldown = 0;
	if (--stopaction_timeout < 0)
		stopaction_timeout = 0;
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
void CooldownStats::reset_stopaction_timeout(int cooldown) {
	stopaction_timeout = std::max(cooldown, stopaction_timeout);
}

bool DerivedStats::operator ==(const DerivedStats & derived) const {
	return damage == derived.damage && power == derived.power
			&& reduction == derived.reduction
			&& resistance == derived.resistance;
}

void CooldownStats::reset_hurt_cooldown(int cooldown) {
	hurt_cooldown = std::max(cooldown, hurt_cooldown);
}

ClassType& ClassStats::class_type() const {
	return game_class_data.at(classid);
}

