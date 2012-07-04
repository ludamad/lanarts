/*
 * GameTextConsole.cpp:
 *  Holds game chat/information messages, as well as descriptions while hovering over items.
 */

#include "../../display/display.h"

#include "../../util/colour_constants.h"

#include "GameTextConsole.h"

GameTextConsole::GameTextConsole(const BBox & bbox) :
		content_already_drawn(false),  bbox(bbox){
}

void GameTextConsole::step(GameState *gs) {
	content_already_drawn = false;
	chat.step(gs);
}

void GameTextConsole::draw_box(GameState *gs) {
	content_already_drawn = true;
	gl_draw_rectangle(bbox.x1, bbox.y1, bbox.width(), bbox.height(),
			COL_CONSOLE_BOX.with_alpha(50));
}

void GameTextConsole::draw(GameState* gs) {
	if (!content_already_drawn) {
		chat.draw(gs);
	}
}
