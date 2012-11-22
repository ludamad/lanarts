/*
 * GameChat.cpp:
 *  Game HUD component for drawing messages.
 *  These messages include things like "A monster has appeared!" and chat messages.
 */

#include <SDL.h>

#include <draw/draw.h>

#include "../draw/colour_constants.h"

#include "../display/display.h"
#include "../gamestate/GameState.h"

#include "GameChat.h"

static void print_dupe_string(const ChatMessage& cm, const font_data& font,
		const Pos& location, float alpha) {
	if (cm.exact_copies > 1)
		gl_printf(font, Colour(0, 191, 255, alpha * 255), location.x,
				location.y, " x%d", cm.exact_copies);
}

void ChatMessage::draw(const font_data& font, float alpha, int x, int y) const {
	Colour sendcol = sender_colour, msgcol = message_colour;
	sendcol.a *= alpha, msgcol.a *= alpha;
	Dim offset(0, 0);
	if (!sender.empty()) {
		offset = gl_printf(font, sendcol, x, y, "%s: ", sender.c_str());
		x += offset.w;
	}
	offset = gl_printf(font, msgcol, x, y, message.c_str());
	x += offset.w;
	print_dupe_string(*this, font, Pos(x, y), alpha);
}

bool ChatMessage::empty() const {
	return sender.empty() && message.empty();
}

void GameChat::add_message(const ChatMessage& cm) {
	bool dupe = false;

	if (!messages.empty()) {
		if (messages.back() == cm) {
			messages.back().exact_copies++;
			dupe = true;
		}
	}

	if (!dupe)
		messages.push_back(cm);

	fade_out = 1.0f;
	fade_out_rate = 0.005f;
}

void GameChat::add_message(const std::string& msg, const Colour& colour) {
	add_message(ChatMessage("", msg, Colour(), colour));
}

bool GameChat::is_typing_message() {
	return is_typing;
}

void GameChat::clear() {
	messages.clear();
}

static const Colour player_colours[] = { COL_BABY_BLUE, COL_PALE_YELLOW,
		COL_PALE_RED, COL_PALE_GREEN, COL_PALE_BLUE, COL_LIGHT_GRAY };
static const int player_colours_n = sizeof(player_colours) / sizeof(Colour);

ChatMessage GameChat::get_field_as_chat_message(GameState* gs,
		bool include_username) const {
	int colour_idx = gs->player_data().local_player_data().net_id
			% player_colours_n;

	ChatMessage typed_message;
	if (include_username) {
		typed_message.sender = gs->game_settings().username;
	}
	typed_message.sender_colour = player_colours[colour_idx];
	typed_message.message = typing_field.text();
	return typed_message;
}

void GameChat::draw_player_chat(GameState* gs) const {
	perf_timer_begin(FUNCNAME);
	const font_data& font = gs->primary_font();
	const int padding = 5;
	int line_sep = font.h + 2;

	int view_w = gs->view().width, view_h = gs->view().height;
	int chat_w = view_w, chat_h = 100;
	int chat_x = 0, chat_y = 0; //h - chat_h - TILE_SIZE;
	int text_x = chat_x + padding, text_y = chat_y + padding;

	gl_draw_rectangle(chat_x, chat_y, chat_w, chat_h,
			COL_CONSOLE_BOX.with_alpha(50 * fade_out));

	bool draw_typed_message = is_typing || !typing_field.empty();

	int start_msg = 0;
	int message_space = chat_h - padding * 2
			- (draw_typed_message ? line_sep : 0);
	int msgs_in_screen = message_space / line_sep;
	if (messages.size() > msgs_in_screen) {
		start_msg = messages.size() - msgs_in_screen;
	}

	for (int i = start_msg; i < messages.size(); i++) {
		messages[i].draw(font, fade_out, text_x, text_y);
		text_y += line_sep;
	}

	if (draw_typed_message) {
		int type_y = chat_y + chat_h - padding - line_sep;
		ldraw::draw_line(Colour(200, 200, 200, fade_out * 180),
				Posf(chat_x, type_y), Posf(chat_x + chat_w, type_y));
		ChatMessage typed_message = get_field_as_chat_message(gs, false);
		typed_message.draw(font, fade_out, text_x, type_y + padding - 1);
	}
	perf_timer_end(FUNCNAME);
}

void GameChat::step(GameState* gs) {
	if (show_chat)
		fade_out = 1.0f;
	else if (fade_out > 0.0f)
		fade_out -= fade_out_rate;
	if (is_typing) {
		typing_field.step();
	}
}
void GameChat::draw(GameState *gs) const {
	if (fade_out > 0.0f) {
		draw_player_chat(gs);
	}
}

void GameChat::toggle_chat(GameState* gs) {
	if (is_typing) {
		if (!typing_field.empty()) {
			ChatMessage typed_message = get_field_as_chat_message(gs, true);
			if (!handle_special_commands(gs, typed_message.message)) {
				add_message(typed_message);
				net_send_chatmessage(gs->net_connection(), typed_message);
			}
		} else {
			show_chat = false;
			fade_out_rate = 0.1f;
		}
		reset_typed_message();
		is_typing = false;
	} else {
		if (!show_chat)
			show_chat = true;
		else if (show_chat)
			is_typing = true;
	}
}
/*Returns whether has handled event completely or not*/
bool GameChat::handle_event(GameState* gs, SDL_Event *event) {
	int view_w = gs->view().width, view_h = gs->view().height;
	int chat_w = view_w, chat_h = 100;
	int chat_x = 0, chat_y = 0; //h - chat_h - TILE_SIZE;

	SDLKey keycode = event->key.keysym.sym;
	SDLMod keymod = event->key.keysym.mod;

	bool did_typing = false;
	if (is_typing) {
		did_typing = (typing_field.handle_event(event));
	}

	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN: {
//		if (show_chat && event->button.button == SDL_BUTTON_LEFT
//				&& gs->mouse_x() < chat_w && gs->mouse_y() < chat_h) {
//			toggle_chat(gs);
//			return true;
//		}
		break;
	}
	case SDL_KEYDOWN: {
		if (keycode == SDLK_RETURN) {
			toggle_chat(gs);
			return true;
		}
		if (is_typing) {
			if (keycode == SDLK_BACKSPACE) {
				if (typing_field.empty()) {
					reset_typed_message();
					is_typing = false;
				}
				return true;
			}
			if (keycode == SDLK_LCTRL || keycode == SDLK_RCTRL) {
				is_typing = false;
				return true;
			}
			if (keycode == SDLK_DELETE) {
				reset_typed_message();
				is_typing = false;
				return true;
			}
		}
		break;
	}
	}
	return did_typing;
}

void GameChat::reset_typed_message() {
	typing_field.clear();
}

static const int TEXT_FIELD_LIMIT = 250;
GameChat::GameChat() :
		typing_field(TEXT_FIELD_LIMIT) {
	show_chat = true;
	fade_out = 1.0f;
	fade_out_rate = 0.05f;
	is_typing = false;
}

void ChatMessage::serialize(SerializeBuffer& serializer) {
	serializer.write(sender);
	serializer.write(message);
	serializer.write(sender_colour);
	serializer.write(message_colour);
}

void ChatMessage::deserialize(SerializeBuffer& serializer) {
	serializer.read(sender);
	serializer.read(message);
	serializer.read(sender_colour);
	serializer.read(message_colour);
}

