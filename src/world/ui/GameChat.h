/*
 * GameChat.h:
 *  Game HUD component for drawing messages.
 *  These messages include things like "A monster has appeared!" and chat messages.
 */

#ifndef GAMECHAT_H_
#define GAMECHAT_H_

#include <SDL.h>
#include <string>
#include <vector>
#include "../../util/game_basic_structs.h"

class GameState;
struct font_data;
struct NetPacket;

/*Handle key repeating, in steps*/
const int INITIAL_REPEAT_STEP_AMNT = 40;
const int NEXT_REPEAT_STEP_AMNT = 5;
const int NEXT_BACKSPACE_STEP_AMNT = 3;

/*Represents a coloured message in chat*/
struct ChatMessage {
	std::string sender, message;
	Colour sender_colour, message_colour;
	int exact_copies;
	ChatMessage(const std::string& sender = std::string(), const std::string& message = std::string(),
			const Colour& sender_colour = Colour(255, 255, 255),
			const Colour& message_colour = Colour(255, 255, 255)) :
			sender(sender), message(message), sender_colour(sender_colour), message_colour(
					message_colour), exact_copies(1) {
	}
	bool operator==(const ChatMessage& cm) const {
		/*Used to determine if message should be added, or 'exact_copies' incremented*/
		return sender == cm.sender && message == cm.message
				&& sender_colour == cm.sender_colour
				&& message_colour == cm.message_colour;
	}
	void draw(const font_data& font, float alpha, int x, int y) const;
	bool empty() const;
	void packet_add(NetPacket& packet);
	void packet_get(NetPacket& packet);
};

class GameChat {
public:
	GameChat();

	void step(GameState* gs);
	void draw(GameState* gs) const;
	void clear();

	void add_message(const ChatMessage& cm);
	void add_message(const std::string& msg,
			const Colour& colour = Colour(255, 255, 255));

	bool is_typing_message();
	/*Returns whether has handled event completely or not*/
	bool handle_event(GameState* gs, SDL_Event *event);
	void toggle_chat(GameState* gs);

private:
	bool handle_special_commands(GameState* gs, const std::string& command);

	void reset_typed_message();
	void draw_player_chat(GameState* gs) const;

	ChatMessage typed_message;

	SDLKey current_key;
	SDLMod current_mod;
	int repeat_steps_left;

	std::vector<ChatMessage> messages;
	bool show_chat, is_typing;
	float fade_out, fade_out_rate;

};

#endif /* GAMECHAT_H_ */
