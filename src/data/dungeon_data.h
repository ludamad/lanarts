/*
 * dungeon_data.h
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#ifndef DUNGEON_DATA_H_
#define DUNGEON_DATA_H_

#include "../procedural/levelgen.h"

enum {
	DNGN_MAIN_BRANCH = 0
};

struct DungeonBranch {
	LevelGenSettings* level_data;
	int nlevels;
	DungeonBranch(LevelGenSettings* level_data, int nlevels):
		level_data(level_data), nlevels(nlevels){
	}
};

extern DungeonBranch game_dungeon_data[];
extern size_t game_dungeon_n;

#endif /* DUNGEON_DATA_H_ */
