/*
 * GameLevelState.h
 *
 *  Created on: Mar 23, 2012
 *      Author: 100397561
 */

#ifndef GAMELEVELSTATE_H_
#define GAMELEVELSTATE_H_
#include <vector>
#include "controllers/MonsterController.h"
#include "controllers/PlayerController.h"
#include "GameInstSet.h"
#include "GameTiles.h"
#include "../procedural/GeneratedLevel.h"

struct GameLevelPortal {
	Pos entrancesqr;
	Pos exitsqr;//0,0 if undecided
	GameLevelPortal(Pos entrance, Pos exit) :
		entrancesqr(entrance), exitsqr(exit){
	}
};

struct GameLevelState{
	GameLevelState(int roomid, int branch, int level, int w, int h, bool is_simulation = false);
	~GameLevelState();

	std::vector<GameLevelPortal> exits, entrances;
	std::vector<Room> rooms;

	int room_within(const Pos& p){
		for (int i = 0; i < rooms.size(); i++){
			int px = p.x/TILE_SIZE, py = p.y/TILE_SIZE;
			const Region& r = rooms[i].room_region;
			if (r.x <= px && r.x + r.w >= px){
				if (r.y <= py && r.y + r.h >= py){
					return i;
				}
			}
		}
		return -1;
	}

	void copy_to(GameLevelState& level) const;
	GameLevelState* clone() const;

	int roomid;//Global room id
	//How many steps a level should be simulated after you leave it
	int steps_left;
	//Game location information
	int branch_number, level_number;

	//Dimensions in pixels
	int width, height;

	//Game world components
	GameTiles tiles;
	GameInstSet inst_set;

	//Game controllers
	MonsterController mc;
	PlayerController pc;

	bool is_simulation;
};

#endif /* GAMELEVELSTATE_H_ */
