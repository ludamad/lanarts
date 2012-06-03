/*
 * class_data.h
 *
 *  Created on: Mar 28, 2012
 *      Author: 100397561
 */

#ifndef CLASS_DATA_H_
#define CLASS_DATA_H_

#include <string>
#include <vector>
#include "../gamestats/Stats.h"

struct ClassType {
	std::string name;
	Stats starting_stats;
	int hp_perlevel, mp_perlevel;
	int str_perlevel, def_perlevel, mag_perlevel;
	float mpregen_perlevel, hpregen_perlevel;
	ClassType(const std::string& name, const Stats& stats,
			int hp_perlevel, int mp_perlevel,
			int str_perlevel, int def_perlevel, int mag_perlevel,
			float mpregen_perlevel, float hpregen_perlevel) :
		name(name), starting_stats(stats),
		hp_perlevel(hp_perlevel), mp_perlevel(mp_perlevel),
		str_perlevel(str_perlevel), def_perlevel(def_perlevel), mag_perlevel(mag_perlevel),
		hpregen_perlevel(hpregen_perlevel),	mpregen_perlevel(mpregen_perlevel){}
};


extern std::vector<ClassType> game_class_data;
int get_class_by_name(const char* name);
#endif /* CLASS_DATA_H_ */
