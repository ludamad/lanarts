/*
 * dungeon_data.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#include "dungeon_data.h"

//Standard width and height
const int STD_W = 128, STD_H = 128;

ItemGenSettings itemfew(1 /*more = stronger items*/, 2,
		4 /*2 to 4 items*/);
ItemGenSettings itemmed(1 /*more = stronger items*/, 5,
		8 /*5 to 8 items*/);
ItemGenSettings itemdefault(1 /*more = stronger items*/, 8,
		16 /*8 to 10 items*/);

RoomGenSettings roomdensevaried(1 /*padding*/, 90 /*amount of rooms*/,
		4 /*min size*/, 30 /*max size*/);
RoomGenSettings roommediumhighpad(3 /*padding*/, 30 /*amount of rooms*/,
		4 /*min size*/, 20 /*max size*/);
RoomGenSettings roomsparsesmall(1 /*padding*/, 15 /*amount of rooms*/,
		4 /*min size*/, 9 /*max size*/);

TunnelGenSettings tunneldefault(1, 3 /*1 to 3 width tunnels*/, 1,
		20 /*1 to 20 tunnels per room*/);
TunnelGenSettings tunnelwide(1, 5 /*1 to 5 width tunnels*/, 1,
		20 /*1 to 20 tunnels per room*/);

FeatureGenSettings featuredefault(3, 3 /*3 stairs up, 3 stairs down*/,
		1 /*Default tileset*/);

FeatureGenSettings featurefirstlevel(0, 3 /*0 stairs up, 3 stairs down*/,
		1 /*Default tileset*/);

/*Min monster index, max monster index, # of monsters*/
EnemyGenSettings enemyfewweak(0, 2, 20);


#define BRANCH(arr) DungeonBranch(arr, sizeof(arr)/sizeof(LevelGenSettings))

static LevelGenSettings mainbranch[] = {

		//Tiny rooms with 5 monsters
		LevelGenSettings(STD_W * 0.3, STD_H * 0.3, itemfew, roomsparsesmall,
				TunnelGenSettings(2, 5 /*2 to 5 width tunnels*/, 5,5 /*5 to 5 tunnels per room*/),
				featurefirstlevel, EnemyGenSettings(0,2,5)),

		LevelGenSettings(
				STD_W * 0.4, STD_H * 0.4, itemmed, roomsparsesmall,
				tunneldefault, featuredefault, EnemyGenSettings(0,3,20)),

		LevelGenSettings(
				STD_W * 0.5, STD_H * 0.5, itemmed, roomsparsesmall,
				tunneldefault, featuredefault, EnemyGenSettings(0,3,25)),

		LevelGenSettings(
				STD_W * 0.6, STD_H * 0.6, itemmed, roommediumhighpad,
				tunneldefault, featuredefault, EnemyGenSettings(0,5,25)),

		LevelGenSettings(
				STD_W * 0.7, STD_H * 0.7, itemmed, roommediumhighpad,
				tunneldefault, featuredefault, EnemyGenSettings(0,5,30)),

		LevelGenSettings(
				STD_W * 0.7, STD_H * 0.7, itemdefault,
				RoomGenSettings(1 /*padding*/, 30 /*amount of rooms*/, 4 /*min size*/, 9 /*max size*/),
				tunneldefault, featuredefault, EnemyGenSettings(0,6,30)),

		LevelGenSettings(
				STD_W * 0.7, STD_H * 0.7, itemdefault,
				RoomGenSettings(1 /*padding*/, 90 /*amount of rooms*/, 4 /*min size*/, 9 /*max size*/),
				tunneldefault, featuredefault, EnemyGenSettings(0,6,30)),

		LevelGenSettings(
				STD_W * 1, STD_H * 1, itemdefault, roommediumhighpad,
				tunnelwide, featuredefault, EnemyGenSettings(2,7,40))
};

DungeonBranch game_dungeon_data[] = { BRANCH(mainbranch) };

size_t game_dungeon_n = sizeof(game_dungeon_data) / sizeof(DungeonBranch);
