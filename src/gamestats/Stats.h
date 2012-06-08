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
	float

	bool hurt(int dmg);
	void heal_fully();
	void heal_hp(float hpgain);
	void heal_mp(float mpgain);
private:
	float hp_regened, mp_regened;
};

/* Stat multiplier, weighted sum*/
struct StatMultiplier {
	float strength, defence, magic, willpower;
	StatMultiplier(float strength, float defence, float magic, float willpower) :
			strength(strength), defence(defence), magic(magic), willpower(
					willpower) {
	}
	float calculate(const CoreStats& stats);
};

/* Derived combat stats */
struct DerivedStats {
	int power, resistance;
	int damage, reduction;
};

/* Core & derived stats after stat bonuses */
struct EffectiveStats {
	CoreStats stats;
	DerivedStats physical, magic;
	float movespeed;
};

/* Cooldown, eg count before a certain action can be done again*/
struct CooldownStats {
	int action_cooldown;
	int pickup_cooldown;
	int rest_cooldown;
	CooldownStats() :
			action_cooldown(0), pickup_cooldown(0), rest_cooldown(0) {
	}
};

/* Represents class related stats */
struct ClassStats {
	class_id classtype;
	int xp, xpneeded, xplevel;
	ClassStats(class_id classtype, int xp, int xpneeded, int xplevel) :
			classtype(classtype), xp(xp), xpneeded(xpneeded), xplevel(xplevel) {
	}
};

/* Represents all the stats used by a combat entity */
struct CombatStats {
	CoreStats core;
	CooldownStats cooldowns;
	ClassStats class_stats;
	Equipment equipment;
	float movespeed;
};

#endif /* STATS_H_ */
