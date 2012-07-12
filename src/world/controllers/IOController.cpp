/*
 * IOController.cpp:
 *  Handles dispatch of events bound to keyboard keys
 */

#include "IOController.h"

IOState::IOState() {
	clear();
}

void IOState::clear() {
	memset(key_down_states, 0, sizeof(key_down_states));
	memset(key_press_states, 0, sizeof(key_press_states));

	mouse_leftdown = false;
	mouse_rightdown = false;

	mouse_leftclick = false;
	mouse_rightclick = false;

	mouse_leftrelease = false;
	mouse_rightrelease = false;

	mouse_didupwheel = false;
	mouse_diddownwheel = false;
}

void IOState::clear_for_step() {
	memset(key_press_states, 0, sizeof(key_press_states));

	mouse_leftclick = false;
	mouse_rightclick = false;

	mouse_leftrelease = false;
	mouse_rightrelease = false;

	mouse_didupwheel = false;
	mouse_diddownwheel = false;

}

IOController::IOController() {

}

void IOController::bind_spell(spell_id spell) {
}

void IOController::unbind_spell(spell_id spell) {
}

void IOController::bind_item(spell_id spell) {
}

void IOController::unbind_item(spell_id spell) {
}

/* Mouse click states */
bool IOController::mouse_right_click() {
	return iostate.mouse_rightclick;
}

bool IOController::mouse_left_down() {
	return iostate.mouse_leftdown;
}

bool IOController::mouse_right_down() {
	return iostate.mouse_rightdown;
}

bool IOController::mouse_left_release() {
	return iostate.mouse_leftrelease;
}

bool IOController::mouse_right_release() {
	return iostate.mouse_rightrelease;
}

bool IOController::mouse_upwheel() {
	return iostate.mouse_didupwheel;
}
bool IOController::mouse_downwheel() {
	return iostate.mouse_diddownwheel;
}
int IOController::key_down_state(int keyval) {
	return iostate.key_down_states[keyval];
}

int IOController::key_press_state(int keyval) {
	return iostate.key_press_states[keyval];
}

bool IOController::mouse_left_click() {
	return iostate.mouse_leftclick;
}

int IOController::mouse_x() {
	return iostate.mousex;
}

int IOController::mouse_y() {
	return iostate.mousey;
}

int IOController::handle_event(SDL_Event *event) {
	int done = 0;

	switch (event->type) {
	case SDL_ACTIVEEVENT:
		break;

	case SDL_KEYDOWN: {
		if (event->key.keysym.sym == SDLK_ESCAPE) {
			done = 1;
		}
		iostate.key_down_states[event->key.keysym.sym] = 1;
		iostate.key_press_states[event->key.keysym.sym] = 1;
		break;
	}
	case SDL_KEYUP: {
		iostate.key_down_states[event->key.keysym.sym] = 0;
		break;
	}
	case SDL_MOUSEBUTTONDOWN: {
		if (event->button.button == SDL_BUTTON_LEFT) {
			iostate.mouse_leftdown = true;
			iostate.mouse_leftclick = true;
		} else if (event->button.button == SDL_BUTTON_RIGHT) {
			iostate.mouse_rightdown = true;
			iostate.mouse_rightclick = true;
		} else if (event->button.button == SDL_BUTTON_WHEELUP) {
			iostate.mouse_didupwheel = true;
		} else if (event->button.button == SDL_BUTTON_WHEELDOWN) {
			iostate.mouse_diddownwheel = true;
		}
		break;
	}
	case SDL_MOUSEBUTTONUP: {
		if (event->button.button == SDL_BUTTON_LEFT) {
			iostate.mouse_leftdown = false;
			iostate.mouse_leftrelease = true;
		} else if (event->button.button == SDL_BUTTON_RIGHT) {
			iostate.mouse_rightdown = false;
			iostate.mouse_rightrelease = true;
		}
		break;
	}
	case SDL_QUIT:
		done = 1;
		break;
	}
	return (done);
}

void IOController::update_iostate(bool resetprev) {
	if (resetprev) {
		iostate.clear_for_step();
	}
	SDL_GetMouseState(&iostate.mousex, &iostate.mousey);
}

bool IOController::query_event(const GameEvent& event) {
	for (int i = 0; i < iostate.active_events.size(); i++) {
		if (event == iostate.active_events[i]) {
			return true;
		}
	}
	return false;
}

