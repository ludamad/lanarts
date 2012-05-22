/*
 * GameInst.cpp
 *
 *  Created on: 2011-09-27
 *      Author: 100397561
 */

#include "GameInst.h"
#include "../GameState.h"
#include "../../display/display.h"

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

//Probably safer to force all implementations to define their own copy_to function
//void GameInst::copy_to(GameInst *inst){
//	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
//	*inst = *this;
//}
