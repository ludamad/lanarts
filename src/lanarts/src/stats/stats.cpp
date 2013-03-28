/**
 * stats.h:
 *  Represents the various kinds of attacks.
 *  Note that EffectiveStats is first created with all static information, ie
 *  without attack choice info, and then attack information is used to complete it.
 */

#include <lcommon/mtwist.h>

#include <luawrap/luawrap.h>

#include "gamestate/GameLogger.h"
#include "items/WeaponEntry.h"

#include "ClassEntry.h"

#include "combat_stats.h"

#include "stats.h"

bool CoreStats::hurt(int dmg) {
	hp -= dmg;
	event_log("CoreStats::hurt dmg=%d, hp=%d of %d\n", dmg, hp, max_hp);

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

	event_log("CoreStats::heal hpgain=%f, hp_regened=%f, hp=%d of %d\n", hpgain, hp_regened, hp, max_hp);

}

void CoreStats::step(const CoreStats& effective_stats, float hp_regen_mod, float mp_regen_mod) {
	heal_hp(effective_stats.hpregen * hp_regen_mod, effective_stats.max_hp);
	heal_mp(effective_stats.mpregen * mp_regen_mod, effective_stats.max_mp);
}

void CoreStats::heal_mp(float mpgain, int maxmp) {
	mp_regened += mpgain;
	if (mp_regened > 0) {
		mp += floor(mp_regened);
		mp_regened -= floor(mp_regened);
	}

	if (mp > maxmp)
		mp = maxmp;

	event_log("CoreStats::healmp mpgain=%f, mp_regened=%f, mp=%d of %d\n", mpgain, mp_regened, mp, max_mp);
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

ClassEntry& ClassStats::class_entry() const {
	return game_class_data.at(classid);
}

CoreStats parse_core_stats(const LuaField& value, bool required) {
	using namespace luawrap;

	CoreStats core;
	if (!required && value.isnil()) {
		return core;
	}
	core.max_mp = defaulted(value, "mp", 0);
	core.max_hp = defaulted(value, "hp", 0);

	core.hp = core.max_hp;
	core.mp = core.max_mp;

	core.hpregen = defaulted(value, "mpregen", 0.0f);
	core.mpregen = defaulted(value, "hpregen", 0.0f);

	core.strength = defaulted(value, "strength", 0);
	core.defence = defaulted(value, "defence", 0);

	core.magic = defaulted(value, "magic", 0);
	core.willpower = defaulted(value, "willpower", 0);
	return core;
}

static Range parse_defaulted_range(const LuaField& value, const Range& r) {
	using namespace luawrap;

	if (value.is<int>()) {
		int i = value.to_int();
		return Range(i, i);
	}
	return defaulted(value, "base", Range(0, 0));
}

/* Accepts nil */
CoreStatMultiplier parse_core_stat_multiplier(const LuaField& value) {
	using namespace luawrap;

	CoreStatMultiplier sm;
	if (value.isnil()) {
		return sm;
	}
	sm.base = parse_defaulted_range(value["base"], Range(0,0));
	sm.strength = defaulted(value, "strength", 0.0f);
	sm.magic = defaulted(value, "magic", 0.0f);
	sm.defence = defaulted(value, "defence", 0.0f);
	sm.willpower = defaulted(value, "willpower", 0.0f);
	return sm;
}

CooldownModifiers parse_cooldown_modifiers(const LuaField& value) {
	using namespace luawrap;

	CooldownModifiers cm;
	cm.melee_cooldown_multiplier = defaulted(value, "melee_cooldown_multiplier",
			1.0f);
	cm.ranged_cooldown_multiplier = defaulted(value,
			"ranged_cooldown_multiplier", 1.0f);
	cm.rest_cooldown_multiplier = defaulted(value, "rest_cooldown_multiplier",
			1.0f);
	cm.spell_cooldown_multiplier = defaulted(value, "spell_cooldown_multiplier",
			1.0f);
	return cm;
}
