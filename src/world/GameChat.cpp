/*
 * GameChat.cpp
 *
 *  Created on: May 27, 2012
 *      Author: 100397561
 */

#include "GameChat.h"
#include "GameState.h"

#include "../display/display.h"

static void draw_player_chat(GameState* gs, float alpha) {
	int w = gs->window_view().width;
	int h = gs->window_view().height;

	GameSettings& settings = gs->game_settings();

	gl_set_drawing_area(0, 0, w, h);

	gl_draw_rectangle(0, 0, w, 100, Colour(180, 180, 255, 50 * alpha));

	int chatx = 10, chaty = 10;
	const char* chatname = settings.username.c_str();
	Pos offset = gl_printf(gs->primary_font(),
			Colour(37, 207, 240, 255 * alpha), chatx, chaty, "%s: ", chatname);
	gl_printf(gs->primary_font(),
			Colour(255, 255, 255, 255 * alpha), chatx + offset.x, chaty, "What's up !");

	Pos offset2 = gl_printf(gs->primary_font(),
			Colour(255, 69, 0, 255 * alpha), chatx, chaty + offset.y, "%s: ", "ciribot");
	gl_printf(gs->primary_font(),
			Colour(255, 255, 255, 255 * alpha), chatx + offset2.x, chaty + offset.y, "nm, u");

}

void GameChat::draw(GameState *gs){
	draw_player_chat(gs, 1.0f);
}

GameChat::GameChat() {
}

