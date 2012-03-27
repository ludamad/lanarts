/*
 * dungeon_data.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#include "dungeon_data.h"

//Standard width and height
const int STD_W = 128, STD_H = 128;

static ItemGenSettings itemfew(1 /*more = stronger items*/, 2,
		4 /*2 to 4 items*/);
static ItemGenSettings itemmed(1 /*more = stronger items*/, 5,
		8 /*5 to 8 items*/);
static ItemGenSettings itemdefault(1 /*more = stronger items*/, 8,
		16 /*8 to 10 items*/);

static RoomGenSettings roomdensevaried(1 /*padding*/, 90 /*amount of rooms*/,
		4 /*min size*/, 30 /*max size*/);
static RoomGenSettings roommediumhighpad(3 /*padding*/, 30 /*amount of rooms*/,
		4 /*min size*/, 20 /*max size*/);
static RoomGenSettings roomuniformsparse(1 /*padding*/, 15 /*amount of rooms*/,
		10 /*min size*/, 12 /*max size*/);

static TunnelGenSettings tunneldefault(1, 3 /*1 to 3 width tunnels*/, 1,
		20 /*1 to 20 tunnels per room*/);
static TunnelGenSettings tunnelwide(1, 5 /*1 to 5 width tunnels*/, 1,
		20 /*1 to 20 tunnels per room*/);

static FeatureGenSettings featuredefault(3, 3 /*3 stairs up, 3 stairs down*/,
		1 /*Default tileset*/);

static FeatureGenSettings featurefirstlevel(0, 3 /*0 stairs up, 3 stairs down*/,
		1 /*Default tileset*/);

/*Min monster index, max monster index, # of monsters*/
//static EnemyGenSettings enemyfewweak(0, 2, 20);


#define BRANCH(arr) DungeonBranch(arr, sizeof(arr)/sizeof(LevelGenSettings))


/*


static LevelGenSettings mainbranch[] = {
		//Level 1
		LevelGenSettings(
				STD_W * 0.3, STD_H * 0.3,
				ItemGenSettings(1, 2,2),
				RoomGenSettings(1, 9, 4,4),
				TunnelGenSettings(1, 2, 2,5),
				featurefirstlevel,
				EnemyGenSettings(0,1,5)),

		//Level 2
		LevelGenSettings(
				STD_W * 0.5, STD_H * 0.5,
				ItemGenSettings(1, 2,9),
				RoomGenSettings(1, 23, 4,6),
				TunnelGenSettings(1, 2, 2,5),
				featuredefault,
				EnemyGenSettings(0,2,15)),

		//Level 3
		LevelGenSettings(
				STD_W * 0.6, STD_H * 0.6,
				ItemGenSettings(1, 2,9),
				RoomGenSettings(1, 20, 8,10),
				TunnelGenSettings(2, 5, 5,10),
				featuredefault,
				EnemyGenSettings(1,4,20)),
		//Level 4
		LevelGenSettings(
				STD_W * 0.7, STD_H * 0.7,
				ItemGenSettings(1, 5,10),
				RoomGenSettings(4, 20, 10,30),
				TunnelGenSettings(2, 6, 2,5),
				featuredefault,
				EnemyGenSettings(1,4,25)),
		//Level 5
		LevelGenSettings(
				STD_W * 0.7, STD_H * 0.7,
				ItemGenSettings(1, 5,10),
				RoomGenSettings(4, 20, 10,30),
				TunnelGenSettings(2, 6, 2,5),
				featuredefault,
				EnemyGenSettings(2,4,25)),

		//Level 6
		LevelGenSettings(
				STD_W * 0.8, STD_H * 0.8,
				ItemGenSettings(1, 2,9),
				RoomGenSettings(4, 20, 10,15),
				TunnelGenSettings(1, 5, 2,20),
				featuredefault,
				EnemyGenSettings(2,5,30)),

		//Level 7
		LevelGenSettings(
				STD_W * 0.8, STD_H * 0.8,
				ItemGenSettings(1, 2,9),
				RoomGenSettings(3, 90, 4,12),
				TunnelGenSettings(1, 3, 9,9),
				featuredefault,
				EnemyGenSettings(2,8,45)),


		//Level 8
		LevelGenSettings(
				STD_W * 0.8, STD_H * 0.8,
				ItemGenSettings(1, 2,9),
				RoomGenSettings(3, 90, 4,12),
				TunnelGenSettings(1, 3, 9,9),
				featuredefault,
				EnemyGenSettings(2,8,50)),
};*/

//
//static LevelGenSettings mainbranch[] = {
//
//		//Tiny rooms with 5 monsters
//		LevelGenSettings(STD_W * 0.3, STD_H * 0.3, itemfew, roomuniformsparse,
//				TunnelGenSettings(2, 5 /*2 to 5 width tunnels*/, 5,5 /*5 to 5 tunnels per room*/),
//				featurefirstlevel, EnemyGenSettings(0,1,5)),
//
//		LevelGenSettings(
//				STD_W * 0.4, STD_H * 0.4, itemmed, roomuniformsparse,
//				tunneldefault, featuredefault, EnemyGenSettings(0,2,20)),
//
//		LevelGenSettings(
//				STD_W * 0.5, STD_H * 0.5, itemmed, roomuniformsparse,
//				tunneldefault, featuredefault, EnemyGenSettings(0,3,25)),
//
//		LevelGenSettings(
//				STD_W * 0.6, STD_H * 0.6, itemmed, roommediumhighpad,
//				tunneldefault, featuredefault, EnemyGenSettings(0,4,28)),
//
//		LevelGenSettings(
//				STD_W * 0.7, STD_H * 0.7, itemmed, roommediumhighpad,
//				tunneldefault, featuredefault, EnemyGenSettings(0,5,30)),
//
//		LevelGenSettings(
//				STD_W * 0.7, STD_H * 0.7, itemdefault,
//				RoomGenSettings(1 /*padding*/, 30 /*amount of rooms*/, 8 /*min size*/, 9 /*max size*/),
//				tunneldefault, featuredefault, EnemyGenSettings(0,6,35)),
//
//		LevelGenSettings(
//				STD_W * 0.7, STD_H * 0.7, itemdefault,
//				RoomGenSettings(1 /*padding*/, 90 /*amount of rooms*/, 8 /*min size*/, 9 /*max size*/),
//				tunneldefault, featuredefault, EnemyGenSettings(0,7,30)),
//
//		LevelGenSettings(
//				STD_W * 1, STD_H * 1, itemdefault, roommediumhighpad,
//				tunnelwide, featuredefault, EnemyGenSettings(2,7,40))
//};

DungeonBranch game_dungeon_data[1] = {  };
std::vector<LevelGenSettings> game_dungeon_yaml;

size_t game_dungeon_n = sizeof(game_dungeon_data) / sizeof(DungeonBranch);
