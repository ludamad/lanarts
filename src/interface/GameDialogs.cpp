#include "GameDialogs.h"
#include <SDL.h>
#include <string>

/* Base class for in-game interactive pop-ups.
 * Used for menus and peaceful areas, eg shops. */
class DialogBox {
public:
	DialogBox(int x, int y, float alpha = 1.0f) :
			x(x), y(y), alpha(alpha) {
		destroyed = false;
	}
	/*Returns whether has handled event completely or not*/
	virtual bool handle_event(GameState* gs, SDL_Event *event) = 0;

	virtual void step(GameState* gs) = 0;
	virtual void draw(GameState* gs) const = 0;
	bool is_destroyed() const {
		return destroyed;
	}

	virtual ~DialogBox() {
	}
protected:
	bool destroyed;
	int x, y;
	float alpha;
};

class MessageDialogBox: public DialogBox {
public:
	MessageDialogBox(const std::string& message, int x, int y, float alpha =
			1.0f) :
			DialogBox(x, y, alpha), message(message) {
	}

	/*Returns whether has handled event completely or not*/
	virtual bool handle_event(GameState* gs, SDL_Event *event) {
		if (event->key.keysym.sym == SDLK_RETURN)
			destroyed = true;
		return false;
	}

	virtual void step(GameState* gs) {

	}
	virtual void draw(GameState* gs) const {

	}

	virtual ~MessageDialogBox() {
	}

protected:
	std::string message;
};

GameDialogs::GameDialogs() {

}

bool GameDialogs::handle_event(GameState* gs, SDL_Event* event) {
	return false;
}

void GameDialogs::step(GameState* gs) {
	if (dialog && dialog->is_destroyed()) {
		delete dialog;
		dialog = NULL;
	}
}

void GameDialogs::draw(GameState* gs) const {
}

GameDialogs::~GameDialogs() {
}

