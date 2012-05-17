/*
 * GameInst.cpp
 *
 *  Created on: 2011-09-27
 *      Author: 100397561
 */

#include "GameInst.h"
#include "../GameState.h"
#include "../../display/display.h"
#include "../../data/lua_data.h"

GameInst::~GameInst() {
}
void GameInst::step(GameState* gs){
}

void GameInst::draw(GameState* gs){
}

void GameInst::init(GameState* gs){

}
void GameInst::deinit(GameState* gs){
	id = 0;
}

