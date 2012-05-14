/*
 * GameLevelState.cpp
 *
 *  Created on: Mar 23, 2012
 *      Author: 100397561
 */

#include "GameLevelState.h"

GameLevelState::GameLevelState(int roomid, int branch, int level, int w, int h) :
	roomid(roomid), steps_left(0), branch_number(branch),
	level_number(level), tiles(w / TILE_SIZE, h / TILE_SIZE), inst_set(w, h){
}

GameLevelState::~GameLevelState() {
}

