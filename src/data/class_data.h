/*
 * class_data.h
 *
 *  Created on: Mar 28, 2012
 *      Author: 100397561
 */

#ifndef CLASS_DATA_H_
#define CLASS_DATA_H_
#include <vector>
#include "../gamestats/Stats.h"

struct ClassType {
	const char* name;
	Stats starting_stats;
	ClassType(const char* name, const Stats& stats) :
		name(name), starting_stats(stats){}
};


extern std::vector<ClassType> game_class_data;
int get_class_by_name(const char* name);
#endif /* CLASS_DATA_H_ */
