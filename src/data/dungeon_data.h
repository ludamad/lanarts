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

struct RoomGenSettings {
	int room_padding;
	Range amount_of_rooms;
	Range size;
	RoomGenSettings() :
			room_padding(1), amount_of_rooms(0,0), size(0, 0) {

	}
};

struct LayoutGenSettings {
	Range width, height;
	TunnelGenSettings tunnels;
	std::vector<RoomGenSettings> rooms;
	bool solid_fill;
	LayoutGenSettings() :
			solid_fill(true) {
	}
};

struct ContentGenSettings {
	ItemGenSettings items;
	FeatureGenSettings features;
	EnemyGenSettings enemies;
};

struct LevelGenSettings {
	std::vector<LayoutGenSettings> layouts;
	ContentGenSettings content;
};


struct DungeonBranch {
	LevelGenSettings* level_data;
	int nlevels;
	DungeonBranch(){
		level_data = NULL;
	}
	DungeonBranch(LevelGenSettings* level_data, int nlevels):
		level_data(level_data), nlevels(nlevels){
	}
};

extern DungeonBranch game_dungeon_data[];
extern std::vector<LevelGenSettings> game_dungeon_yaml;
extern size_t game_dungeon_n;

#endif /* DUNGEON_DATA_H_ */
