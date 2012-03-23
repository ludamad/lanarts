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
	GameLevelState(int branch, int level, int w, int h);
	~GameLevelState();

	std::vector<GameLevelPortal> exits, entrances;

	//Game location information
	int branch_number, level_number;

	//Game world components
	GameTiles tiles;
	GameInstSet inst_set;

	//Game controllers
	MonsterController mc;
	PlayerController pc;
};

#endif /* GAMELEVELSTATE_H_ */
