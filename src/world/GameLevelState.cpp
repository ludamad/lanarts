/*
 * GameLevelState.cpp
 *
 *  Created on: Mar 23, 2012
 *      Author: 100397561
 */

#include "GameLevelState.h"

GameLevelState::GameLevelState(int roomid, int branch, int level, int w, int h,
		bool is_simulation) :
		roomid(roomid), steps_left(0), branch_number(branch), level_number(
				level), width(w), height(h), tiles(w / TILE_SIZE, h / TILE_SIZE), inst_set(w, h), is_simulation(
				is_simulation) {
}

GameLevelState::~GameLevelState() {
}

void GameLevelState::copy_to(GameLevelState & level) const {
	level.branch_number = this->branch_number;
	level.entrances = this->entrances;//Copy exits&entrances just in case
	level.exits = this->exits;//However we will typically copy_to just to synch
	this->inst_set.copy_to(level.inst_set);
	level.is_simulation = this->is_simulation;
	level.level_number = this->level_number;
	tiles.copy_to(level.tiles);
	this->mc.partial_copy_to(level.mc);
	level.mc.finish_copy(&level);
	this->pc.copy_to(level.pc);
	level.is_simulation = this->is_simulation;
}

GameLevelState* GameLevelState::clone() const{
	GameLevelState* state = new GameLevelState(roomid, branch_number, level_number, width, height, is_simulation);
	copy_to(*state);
	return state;
}



