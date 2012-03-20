/*
 * dungeon_data.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#include "dungeon_data.h"

//Standard width and height
const int STD_W = 128, STD_H = 128;
ItemGenSettings itemdefault(1 /*more = stronger items*/, 5, 10 /*5 to 10 items*/);
RoomGenSettings roomdefault(1 /*padding*/, 90 /*amount of rooms*/, 4 /*min size*/, 30 /*max size*/);
TunnelGenSettings tunneldefault(1,2 /*1 to 2 width tunnels*/, 1, 20 /*1 to 20 tunnels per room*/);
FeatureGenSettings featuredefault(3,3 /*3 stairs up, 3 stairs down*/);

#define BRANCH(arr) DungeonBranch(arr, sizeof(arr)/sizeof(LevelGenSettings))

static LevelGenSettings mainbranch[] = {
		LevelGenSettings(STD_W*0.5, STD_H*0.5, itemdefault, roomdefault, tunneldefault, featuredefault),
		LevelGenSettings(STD_W*0.6, STD_H*0.6, itemdefault, roomdefault, tunneldefault, featuredefault),
		LevelGenSettings(STD_W*0.7, STD_H*0.7, itemdefault, roomdefault, tunneldefault, featuredefault),
};

DungeonBranch game_dungeon_data[] = {
		BRANCH(mainbranch)
};

size_t game_dungeon_n = sizeof(game_dungeon_data)/sizeof(DungeonBranch);
