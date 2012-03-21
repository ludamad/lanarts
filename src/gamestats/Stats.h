#ifndef STATS_H_
#define STATS_H_

struct Stats {
	int hp, max_hp;
	int mp, max_mp;
	int cooldown, start_cooldown;
	int melee_reach, bulletspeed;
	int xp, xpneeded;
	Stats(int hp, int mp, int cooldown, int mreach, int bspeed = 7) :
			hp(hp), max_hp(hp), mp(mp), max_mp(mp), cooldown(cooldown),
			start_cooldown(cooldown), melee_reach(mreach), bulletspeed(bspeed), xp(0),
			xpneeded(100) {
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
};

#endif /* STATS_H_ */
