/**
 * stats.h:
 *  Represents the various kinds of attacks.
 *  Note that EffectiveStats is first created from CoreStats with all static
 *  information, ie  without attack choice info, and then EffectiveAttackStats
 *  is finally created once an attack is chosen.
 */

#ifndef STATS_H_
#define STATS_H_

#include <cmath>
#include <cstring>
#include <map>

#include "lanarts_defines.h"

#include <lcommon/SerializeBuffer.h>
#include "AllowedActions.h"
#include "stats/SpellsKnown.h"

struct AttackStats;
struct ClassEntry;
class MTwist;
class LuaField;
struct GameState;

/* Core combat stats*/
struct CoreStats {
	int hp = 0, max_hp = 0;
	int mp = 0, max_mp = 0;
	int strength = 0, defence = 0, magic = 0, willpower = 0;
	float hpregen = 0, mpregen = 0;
	// Values < 0
	float hp_regened = 0, mp_regened = 0;
    float spell_velocity_multiplier = 1.0;

	void step(const CoreStats& effective_stats, float hp_regen_mod = 1, float mp_regen_mod = 1);

	bool hurt(int dmg);
	void heal_fully();
	void heal_hp(float hpgain, int maxhp);
	void heal_mp(float mpgain, int maxmp);
	void apply_as_bonus(const CoreStats& bonus_stats);
};

CoreStats parse_core_stats(const LuaField& value, bool required = false);

/* Stat multiplier, weighted sum*/
struct CoreStatMultiplier {
	Range base;
	float strength, defence, magic, willpower;
	CoreStatMultiplier(int base = 0, float strength = 0, float defence = 0,
			float magic = 0, float willpower = 0) :
			base(base, base), strength(strength), defence(defence), magic(
					magic), willpower(willpower) {
	}
	bool is_empty() {
		return base.min == 0 && base.max == 0 && strength == 0 && defence == 0
				&& magic == 0 && willpower == 0;
	}
	Range calculate_range(const CoreStats& stats) const;
	float calculate(MTwist& mt, const CoreStats& stats) const;
};

/* Accepts nil */
CoreStatMultiplier parse_core_stat_multiplier(const LuaField& value);

/* Stats related to a chosen attack */
struct EffectiveAttackStats {
	float damage = 0.0f, power = 0.0f, cooldown = 0.0f;
	float magic_percentage = 0.0f;
	float type_multiplier = 1.0f; // For super effective / not very effective
	float physical_percentage() const {
		return 1.0f - magic_percentage;
	}
};

struct CooldownModifiers {
	float rest_cooldown_multiplier;
	float spell_cooldown_multiplier;
	float melee_cooldown_multiplier;
	float ranged_cooldown_multiplier;
	CooldownModifiers() :
			rest_cooldown_multiplier(1.0f), spell_cooldown_multiplier(1.0f), melee_cooldown_multiplier(
					1.0f), ranged_cooldown_multiplier(1.0f) {
	}
	void apply(float mult) {

		rest_cooldown_multiplier *= mult;
		spell_cooldown_multiplier *= mult;
		melee_cooldown_multiplier *= mult;
		ranged_cooldown_multiplier *= mult;
	}
	void apply(const CooldownModifiers& cooldown_modifiers) {
		rest_cooldown_multiplier *= cooldown_modifiers.rest_cooldown_multiplier;
		spell_cooldown_multiplier *=
				cooldown_modifiers.spell_cooldown_multiplier;
		melee_cooldown_multiplier *=
				cooldown_modifiers.melee_cooldown_multiplier;
		ranged_cooldown_multiplier *=
				cooldown_modifiers.ranged_cooldown_multiplier;
	}
};

CooldownModifiers parse_cooldown_modifiers(const LuaField& value);
SpellsKnown parse_spells_known(const LuaField& value);

/* Core & derived stats after stat & item properties */
struct EffectiveStats {
	CoreStats core;
	CooldownModifiers cooldown_modifiers;
	float cooldown_mult, movespeed;
	AllowedActions allowed_actions;
    SpellsKnown spells;
	EffectiveStats() :
			cooldown_mult(1.0f), movespeed(0.0f) {
	}

	EffectiveAttackStats with_attack(MTwist& mt,
			const AttackStats& attack) const;
    void serialize(SerializeBuffer& serializer) {
        SERIALIZE_POD_REGION(serializer, this, core, allowed_actions);
        spells.serialize(serializer);
    }
    void deserialize(SerializeBuffer& serializer) {
        DESERIALIZE_POD_REGION(serializer, this, core, allowed_actions);
        spells.deserialize(serializer);
    }
};

/* Cooldown, eg count before a certain action can be done again*/
struct CooldownStats {
	int action_cooldown = 0;
	int pickup_cooldown = 0;
	int rest_cooldown = 0;
	int hurt_cooldown = 0;
	int stopaction_timeout = 0;
	std::map<spell_id, int> spell_cooldowns;

	void step(bool is_resting);

	bool can_rest() {
		return rest_cooldown <= 0;
	}
	bool can_pickup() {
		return pickup_cooldown <= 0;
	}
	bool can_doaction() {
		return action_cooldown <= 0;
	}
	bool is_hurting() {
		return hurt_cooldown > 0;
	}
	bool can_do_stopaction() {
		return stopaction_timeout <= 0;
	}

    void serialize(GameState* gs, SerializeBuffer& serializer);
    void deserialize(GameState* gs, SerializeBuffer& serializer);

	void reset_action_cooldown(int cooldown);
	void reset_pickup_cooldown(int cooldown);
	void reset_rest_cooldown(int cooldown);
	void reset_hurt_cooldown(int cooldown);
	void reset_stopaction_timeout(int cooldown);
};

/* Represents class related stats */
struct ClassStats {
	class_id classid;
	int xp, xpneeded, xplevel;
	ClassStats(class_id classtype = -1, int xplevel = 0, int xp = 0,
			int xpneeded = 0) :
			classid(classtype), xp(xp), xpneeded(xpneeded), xplevel(xplevel) {
	}
	bool has_class() const {
		return classid >= 0;
	}
	ClassEntry& class_entry() const;
};

#endif /* STATS_H_ */
