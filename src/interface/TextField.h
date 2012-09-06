/*
 * TextField.h:
 *  Interface component for entering text.
 *  This includes stuff like entering name before play, etc.
 */

#ifndef TEXTFIELD_H_
#define TEXTFIELD_H_

#include <string>
#include <SDL.h>

class GameState;

class TextField {
public:
	TextField(const std::string& default_text = std::string()) :
			_text(default_text) {
	}
	// Get current contents
	std::string& text() {
		return _text;
	}
	/*Returns whether has handled event*/
	bool handle_event(SDL_Event *event);
private:
	SDLKey current_key;
	SDLMod current_mod;

	std::string _text;
	int repeat_steps_left;
};

#endif /* TEXTFIELD_H_ */
