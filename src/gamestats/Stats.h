#ifndef STATS_H_
#define STATS_H_

struct Attack  {//Currently for melee & ranged
	bool canuse;
	int damage;
	int range, cooldown;
	int projectile_sprite;
	int projectile_speed;

	Attack(bool canuse = false, int damage = 0, int range = 0, int cooldown = 0, int spr = 0, int bspeed = 0 ) :
		canuse(canuse), damage(damage), range(range), cooldown(cooldown), projectile_sprite(spr), projectile_speed(bspeed){
	}
};
struct Stats {
	float movespeed;
	int hp, max_hp;
	int mp, max_mp;
	int cooldown;

	Attack melee, ranged;

	int xp, xpneeded, xplevel;
	Stats(float speed, int hp, int mp, const Attack& melee, const Attack& ranged) :
			movespeed(speed),
			hp(hp), max_hp(hp), mp(mp), max_mp(mp),
			cooldown(0),
			melee(melee), ranged(ranged),
			xp(0), xpneeded(100), xplevel(1) {
	}
	void step() {
		cooldown--;
		if (cooldown < 0)
			cooldown = 0;
	}
	bool has_cooldown() {
		return cooldown > 0;
	}
	void reset_melee_cooldown() {
		cooldown = melee.cooldown;
	}
	void reset_ranged_cooldown() {
		cooldown = ranged.cooldown;
	}
	void gain_level(){
		hp += 15;
		max_hp += 15;
		mp += 10;
		max_mp += 10;
		melee.damage += 2;
		ranged.damage += 2;
		xplevel ++;
	}
	void gain_xp(int amnt){
		xp += amnt;
		if (xp >= xpneeded){
			xp -= xpneeded;
			xpneeded = (xplevel)*50;
			gain_level();
		}

	}
};

#endif /* STATS_H_ */
