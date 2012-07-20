/*
 * GameTextConsole.h:
 *  Holds game chat/information messages, as well as descriptions while hovering over items.
 */

#ifndef GAMETEXTCONSOLE_H_
#define GAMETEXTCONSOLE_H_

#include "GameChat.h"

class GameTextConsole {
public:
	GameTextConsole(const BBox& bbox);
	GameChat& game_chat() {
		return chat;
	}
	const BBox& bounding_box() {
		return bbox;
	}
	bool has_content_already() {
		return content_already_drawn;
	}
	void mark_as_drawn() {
		content_already_drawn = true;
	}
	void step(GameState* gs);
	void draw_box(GameState* gs);
	void draw(GameState* gs);
private:
	bool content_already_drawn;
	BBox bbox;
	GameChat chat;
};

#endif /* GAMETEXTCONSOLE_H_ */
