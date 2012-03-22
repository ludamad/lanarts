#ifndef STATS_H_
#define STATS_H_

struct Stats {
	float movespeed;
	int hp, max_hp;
	int mp, max_mp;
	bool can_melee, can_range;//TODO: Make much more fleshed out with attack profiles
	int melee_dmg, range_dmg;
	int cooldown, start_cooldown;
	int melee_reach, range, bulletspeed;
	int xp, xpneeded, xplevel;
	Stats(float speed, int hp, int mp, bool can_melee, bool can_range, int melee_dmg, int range_dmg,
			int cooldown, int mreach, int range, int bspeed = 7) :
			movespeed(speed),
			hp(hp), max_hp(hp), mp(mp), max_mp(mp),
			can_melee(can_melee), can_range(can_range),
			melee_dmg(melee_dmg), range_dmg(range_dmg),
			cooldown(cooldown), start_cooldown(cooldown),
			melee_reach(mreach), range(range),
			bulletspeed(bspeed),
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
	void reset_cooldown() {
		cooldown = start_cooldown;
	}
	void gain_xp(int amnt){
		xp += amnt;
		if (xp >= xpneeded){
			xp -= xpneeded;
			xpneeded = xplevel*100;
			xplevel ++;
		}

	}
};

#endif /* STATS_H_ */
