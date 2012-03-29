#ifndef STATS_H_
#define STATS_H_

#include <cmath>
#include <cstring>

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
	float movespeed;
	int hp, max_hp;
	int mp, max_mp;
	float hpregen, mpregen;
	int cooldown;
	int hurt_cooldown;
	float hp_regened, mp_regened;

	Attack melee, ranged;

	int xp, xpneeded, xplevel;

	int strength, dexterity, magic;
	Stats() {
		memset(this, 0, sizeof(Stats));
	}
	Stats(float speed, int hp, int mp, int strength,
			int dexterity, int magic, const Attack& melee,
			const Attack& ranged);
	void step();

	bool has_cooldown();

	float hurt_alpha();
	void set_hurt_cooldown();
	void reset_melee_cooldown();

	void reset_ranged_cooldown();

	void reset_melee_cooldown(const Stats & effectivestats);

	void reset_ranged_cooldown(const Stats & effectivestats);

	bool hurt(int dmg);

	void gain_level();
	void gain_xp(int amnt);

	void raise_hp(float hpgain);
	void raise_mp(float mpgain);
};

#endif /* STATS_H_ */
