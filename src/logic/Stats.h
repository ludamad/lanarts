#ifndef STATS_H_
#define STATS_H_

struct Stats {
	int hp, max_hp;
	int mp, max_mp;
	Stats(int hp, int mp) :
		hp(hp), max_hp(hp), mp(mp), max_mp(mp){}
};


#endif /* STATS_H_ */
