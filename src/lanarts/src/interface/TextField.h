/*
 * TextField.h:
 *  Interface component for entering text.
 *  This includes stuff like entering name before play, etc.
 */

#ifndef TEXTFIELD_H_
#define TEXTFIELD_H_

#include <string>
#include <SDL.h>

#include <lcommon/Timer.h>

class GameState;

class TextField {
public:
	TextField(int max_length, const std::string& default_text = std::string());

	void set_text(const std::string& txt) {
		_text = txt;
	}
    const std::string& text() const {
        return _text;
    }

    bool empty() const {
    	return _text.empty();
    }
    void step();
    void clear();
    void clear_keystate();
	bool handle_event(SDL_Event *event);
private:
	bool _has_repeat_cooldown();
	void _reset_repeat_cooldown(int cooldownms);
	SDLKey _current_key;
	SDLMod _current_mod;

	std::string _text;
	int _max_length;
	int _repeat_cooldown;
	Timer _repeat_timer;
};

#endif /* TEXTFIELD_H_ */
