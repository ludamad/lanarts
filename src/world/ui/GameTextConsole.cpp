/*
 * GameTextConsole.cpp:
 *  Holds game chat/information messages, as well as descriptions while hovering over items.
 */

#include "GameTextConsole.h"

GameTextConsole::GameTextConsole(const BBox & bbox) {
}

void GameTextConsole::step(GameState *gs) {
	content_already_drawn = false;
}

void GameTextConsole::draw(GameState *gs) {
	if (!content_already_drawn) {
		chat.draw(gs);
	}
}
