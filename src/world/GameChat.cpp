/*
 * GameChat.cpp
 *
 *  Created on: May 27, 2012
 *      Author: 100397561
 */

#include "GameChat.h"
#include "GameState.h"

#include "../display/display.h"

void GameChat::draw_player_chat(GameState* gs) {
	int w = gs->window_view().width, h = gs->window_view().height;
	int chat_w = w, chat_h = 100;
	int chat_x = 0, chat_y = h - chat_h - TILE_SIZE;
	int textx = chat_x + 10, text_y = chat_y + 10;

	int alpha_channel = 255 * fade_out;

	gl_set_drawing_area(0, 0, w, h);

	gl_draw_rectangle(chat_x, chat_y, chat_w, chat_h, Colour(180, 180, 255, 50 * fade_out));

	for (int i = 0; i < messages.size(); i++) {
		ChatMessage& cm = messages[i];
		Colour sender_colour = cm.sender_colour, message_colour =
				cm.message_colour;
		sender_colour.a = alpha_channel, message_colour.a = alpha_channel;

		Pos offset = gl_printf(gs->primary_font(), sender_colour, textx, text_y,
				"%s: ", cm.sender.c_str());
		offset = gl_printf(gs->primary_font(), message_colour, textx + offset.x, text_y,
				cm.message.c_str());
		text_y += offset.y;

	}

}

void GameChat::step(GameState *gs) {
	if (gs->key_press_state(SDLK_c)) {
		show_chat = !show_chat;
	}
	if (show_chat)
		fade_out = 1.0f;
	else if (fade_out > 0.0f)
		fade_out -= 0.05f;
}
void GameChat::draw(GameState *gs) {
	if (fade_out > 0.0f)
		draw_player_chat(gs);
}

GameChat::GameChat() {
	show_chat = true;
	fade_out = 1.0f;
	messages.push_back(
			ChatMessage("ludamad", "What's up!?\nGo eff off", Colour(37, 207, 240)));
	messages.push_back(ChatMessage("ciribot", "nm u", Colour(255, 69, 0)));
}

