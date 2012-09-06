/*
 * TextField.cpp:
 *  Interface component for entering text.
 *  This includes stuff like entering name before play, etc.
 */

#include <SDL.h>

#include "TextField.h"

static char keycode_to_char(SDLKey keycode, SDLMod keymod) {
	const char DIGIT_SYMBOLS[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*',
			'(' };
	const char MISC_SYMBOLS[][2] = { { '`', '~' }, { ',', '<' }, { '.', '>' }, {
			'/', '?' }, { ';', ':' }, { '\'', '"' }, { '[', '{' }, { ']', '}' },
			{ '\\', '|' }, { '-', '_' }, { '=', '+' } };
	bool hitcaps = (keymod & KMOD_CAPS);
	bool hitshift = (keymod & (KMOD_LSHIFT | KMOD_RSHIFT));
	if ((hitcaps != hitshift) && isalpha(keycode)) {
		return toupper(keycode);
	} else if (hitshift) {
		if (isdigit(keycode))
			return DIGIT_SYMBOLS[keycode - '0'];
		for (int i = 0; i < sizeof(MISC_SYMBOLS) / sizeof(char) / 2; i++) {
			if (keycode == MISC_SYMBOLS[i][0])
				return MISC_SYMBOLS[i][1];
		}
	}
	return keycode;
}
static bool is_typeable_keycode(SDLKey keycode) {
	return (keycode >= SDLK_SPACE && keycode <= SDLK_z);
}

/*Returns whether has handled event*/
bool TextField::handle_event(SDL_Event* event) {
	SDLKey keycode = event->key.keysym.sym;
	SDLMod keymod = event->key.keysym.mod;
	current_mod = keymod;
	switch (event->type) {
	case SDL_KEYUP: {
		/*Since the key-up isnt truly an action, we respond but pretend we didn't handle it*/
		if (current_key == keycode) {
			current_key = SDLK_FIRST;
		}
		break;
	}
	case SDL_KEYDOWN: {
		if (is_typeable_keycode(keycode)) {
			_text += keycode_to_char(keycode, keymod);
			if (current_key != keycode) {
				current_key = keycode;
				repeat_steps_left = INITIAL_REPEAT_STEP_AMNT;
			}
			return true;
		}
		if (keycode == SDLK_BACKSPACE) {
			if (!_text.empty()) {
				_text.resize(_text.size() - 1);
			}
			if (current_key != keycode) {
				current_key = keycode;
				repeat_steps_left = INITIAL_REPEAT_STEP_AMNT;
			}
			return true;
		}
		if (keycode == SDLK_DELETE) {
			_text.clear();
			return true;
		}
		break;
	}
	}
	return false;
}

