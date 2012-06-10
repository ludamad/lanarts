#ifndef STATS_H_
#define STATS_H_

#include <cmath>
#include <cstring>

#include "../util/game_basic_structs.h"

struct StatModifier;
class MTwist;
struct Attack { //Currently for melee & ranged
	bool canuse;
	int damage;
	int range, cooldown;
	int attack_sprite;
	int projectile_speed;
	bool isprojectile;

	Attack(bool canuse = false, int damage = 0, int range = 0, int cooldown = 0,
			int spr = 0, int bspeed = 0) :
			canuse(canuse), damage(damage), range(range), cooldown(cooldown), attack_sprite(
					spr), projectile_speed(bspeed), isprojectile(false) {
	}
};
struct Stats {
	int classtype;
	float movespeed;
	int hp, max_hp;
	int mp, max_mp;
	float hpregen, mpregen;
	int cooldown;
	int hurt_cooldown;
	float hp_regened, mp_regened;

	Attack meleeatk, magicatk;

	int xp, xpneeded, xplevel;

	int strength, defence, magic;
	Stats() {
		memset(this, 0, sizeof(Stats));
	}
	Stats(float speed, int hp, int mp, int strength, int defence, int magic,
			const Attack& melee, const Attack& ranged);
	void step();

	bool has_cooldown();

	float hurt_alpha();
	void set_hurt_cooldown();
	void reset_melee_cooldown();

	void reset_ranged_cooldown();

	void reset_melee_cooldown(const Stats & effectivestats);

	void reset_ranged_cooldown(const Stats & effectivestats);

	bool hurt(int dmg);

	void heal_fully();
	void raise_hp(float hpgain);
	void raise_mp(float mpgain);

	int calculate_statmod_damage(MTwist& mt, StatModifier& sm);
	int calculate_melee_damage(MTwist& mt, int weapon_type);
	int calculate_ranged_damage(MTwist& mt, int weapon_type,
			int projectile_type);
	int calculate_spell_damage(MTwist& mt, int spell_type);

	void gain_level();
	int gain_xp(int amnt);

};

/* Core combat stats*/
struct CoreStats {
	int hp, max_hp;
	int mp, max_mp;
	int strength, defence, magic, willpower;
	float hpregen, mpregen;

	CoreStats() :
			hp(0), max_hp(0), mp(0), max_mp(0), strength(0), defence(0), magic(
					0), willpower(0), hpregen(0), mpregen(0), hp_regened(0), mp_regened(
					0) {

	}

	void step();

	bool hurt(int dmg);
	void heal_fully();
	void heal_hp(float hpgain);
	void heal_mp(float mpgain);
private:
	float hp_regened, mp_regened;
};

/* Stat multiplier, weighted sum*/
struct CoreStatMultiplier {
	Range base;
	float strength, defence, magic, willpower;
	CoreStatMultiplier(int base = 0, float strength = 0, float defence = 0, float magic = 0, float willpower = 0) :
			strength(strength), defence(defence), magic(magic), willpower(
					willpower) {
	}
	float calculate(MTwist& mt, const CoreStats& stats);
};

/* Derived combat stats
 * Things like cooldown heavily influenced by current attack */
struct DerivedStats {
	int power, resistance;
	int damage, reduction;
	int cooldown;
	DerivedStats(int power = 0, int resistance = 0, int damage = 0,
			int reduction = 0, int cooldown = 0) :
			power(power), resistance(resistance), damage(damage), reduction(
					reduction), cooldown(0) {
	}

	bool operator==(const DerivedStats& derived) const;
};

/* Core & derived stats after stat bonuses
 * */
struct EffectiveStats {
	CoreStats core;
	DerivedStats physical, magic;
	float movespeed;
	EffectiveStats() :
			movespeed(0.0f) {
	}
};

/* Cooldown, eg count before a certain action can be done again*/
struct CooldownStats {
	int action_cooldown;
	int pickup_cooldown;
	int rest_cooldown;
	int hurt_cooldown;

	CooldownStats() :
			action_cooldown(0), pickup_cooldown(0), rest_cooldown(0), hurt_cooldown(
					0) {
	}

	void step();

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

	void reset_action_cooldown(int cooldown);
	void reset_pickup_cooldown(int cooldown);
	void reset_rest_cooldown(int cooldown);
	void reset_hurt_cooldown(int cooldown);
};

/* Represents class related stats */
struct ClassStats {
	class_id classtype;
	int xp, xpneeded, xplevel;
	ClassStats(class_id classtype = -1, int xplevel = 0, int xp = 0,
			int xpneeded = 0) :
			classtype(classtype), xp(xp), xpneeded(xpneeded), xplevel(xplevel) {
	}
};

#endif /* STATS_H_ */
