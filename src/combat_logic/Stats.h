#ifndef STATS_H_
#define STATS_H_

struct Stats {
	int cooldown, start_cooldown;
	int melee_reach;
	int hp, max_hp;
	int mp, max_mp;
	Stats(int hp, int mp, int mreach) :
		cooldown(10), start_cooldown(10), melee_reach(mreach),
		hp(hp), max_hp(hp), mp(mp), max_mp(mp){}
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
