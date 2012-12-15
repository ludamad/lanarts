/*
 * GameTextConsole.cpp:
 *  Holds game chat/information messages, as well as descriptions while hovering over items.
 */

#include <draw/draw.h>

#include "../draw/colour_constants.h"

#include "../display/display.h"

#include "GameTextConsole.h"

GameTextConsole::GameTextConsole(const BBox & bbox) :
		content_already_drawn(false), bbox(bbox) {
}

void GameTextConsole::step(GameState *gs) {
	content_already_drawn = false;
	chat.step(gs);
}

void GameTextConsole::draw_box(GameState *gs) {
	content_already_drawn = true;
	ldraw::draw_rectangle(COL_CONSOLE_BOX.alpha(50), bbox);
}

void GameTextConsole::draw(GameState* gs) {
	if (!content_already_drawn) {
		chat.draw(gs);
	}
}
