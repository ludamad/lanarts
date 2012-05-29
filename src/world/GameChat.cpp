/*
 * GameChat.cpp
 *
 *  Created on: May 27, 2012
 *      Author: 100397561
 */

#include "GameChat.h"
#include "GameState.h"

#include "../display/display.h"

void GameChat::add_message(const ChatMessage& cm) {
	messages.push_back(cm);
	fade_out = 1.0f;
	fade_out_rate = 0.0025f;
}

void GameChat::add_message(const std::string& msg, const Colour& colour) {
	add_message(ChatMessage("", msg, Colour(), colour));
}

void GameChat::draw_player_chat(GameState* gs) {
	int w = gs->window_view().width, h = gs->window_view().height;
	int chat_w = w, chat_h = 100;
	int chat_x = 0, chat_y = 0; //h - chat_h - TILE_SIZE;
	int textx = chat_x + 10, text_y = chat_y + 10;

	int alpha_channel = 255 * fade_out;

	gl_set_drawing_area(0, 0, w, h);

	gl_draw_rectangle(chat_x, chat_y, chat_w, chat_h,
			Colour(180, 180, 255, 50 * fade_out));
	const font_data& font = gs->primary_font();
	int start_msg = 0;
	int msgs_in_screen = chat_h / (font.h + 1);
	if (messages.size() > msgs_in_screen - 2) {
		start_msg = messages.size() - msgs_in_screen - 2;
//		text_y += int(start_msg * (font.h + 1)) % chat_h;
	}

	for (int i = start_msg; i < messages.size(); i++) {
		ChatMessage& cm = messages[i];
		Colour sender_colour = cm.sender_colour, message_colour =
				cm.message_colour;
		sender_colour.a = alpha_channel, message_colour.a = alpha_channel;
		Pos offset(0, 0);
		if (!cm.sender.empty())
			offset = gl_printf(font, sender_colour, textx, text_y, "%s: ",
					cm.sender.c_str());
		offset = gl_printf(font, message_colour, textx + offset.x, text_y,
				cm.message.c_str());
		text_y += offset.y;

	}

}

void GameChat::step(GameState *gs) {
	if (gs->key_press_state(SDLK_c)) {
		show_chat = !show_chat;
		fade_out_rate = 0.05f;
	}
	if (show_chat)
		fade_out = 1.0f;
	else if (fade_out > 0.0f)
		fade_out -= fade_out_rate;
}
void GameChat::draw(GameState *gs) {
	if (fade_out > 0.0f)
		draw_player_chat(gs);
}

GameChat::GameChat() {
	show_chat = false;
	fade_out = 0.0f;
	fade_out_rate = 0.05f;
//	messages.push_back(
//			ChatMessage("ludamad", "What's up!?\nGo eff off",
//					Colour(37, 207, 240)));
//	messages.push_back(ChatMessage("ciribot", "nm u", Colour(255, 69, 0)));
}

