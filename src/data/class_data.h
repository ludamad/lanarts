/*
 * class_data.h:
 *  Contains data related to class progression
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
	ClassType(){
	}
};

extern std::vector<ClassType> game_class_data;
int get_class_by_name(const char* name);

#endif /* CLASS_DATA_H_ */
