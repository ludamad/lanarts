#ifndef STATS_H_
#define STATS_H_

struct Stats {
	int cooldown, start_cooldown;
	int melee_reach;
	int hp, max_hp;
	int mp, max_mp;
	int xp, xpneeded;
	Stats(int hp, int mp, int mreach) :
		cooldown(50), start_cooldown(50), melee_reach(mreach),
		hp(hp), max_hp(hp), mp(mp), max_mp(mp), xp(0), xpneeded(100){}
	void step(){
		cooldown--;
		if (cooldown < 0) cooldown = 0;
	}
	bool has_cooldown(){
		return cooldown > 0;
	}
	void reset_cooldown(){
		cooldown = start_cooldown;
	}
};


#endif /* STATS_H_ */
