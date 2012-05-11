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
	delete lua_hooks;
}
void GameInst::step(GameState* gs){
	if (lua_hooks)
		luadata_step_event(gs->get_luastate(), *lua_hooks, id);
}

void GameInst::draw(GameState* gs){
	if (lua_hooks)
		luadata_draw_event(gs->get_luastate(), *lua_hooks, id);
}

void GameInst::init(GameState* gs){

}
void GameInst::deinit(GameState* gs){
	id = 0;
}

