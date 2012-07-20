/*
 * IOController.cpp:
 *  Handles dispatch of events bound to keyboard keys
 */

#include "IOController.h"

bool IOEventTrigger::should_trigger(IOEventTrigger::trigger_t trigger,
		SDLKey trigger_key, SDLMod mod, bool holding_key) {
	if (!this->trigger_continuously && holding_key) {
		return false;
	}
	if ((this->trigger && this->trigger != trigger)
			|| (this->trigger_key && this->trigger_key != trigger_key)) {
		return false;
	}
	if (this->mod_required && !(this->mod_required & mod)) {
		return false;
	}
	if (this->mod_rejected && (this->mod_rejected & mod)) {
		return false;
	}
	return true;
}

IOState::IOState() {
	clear();
}

void IOState::clear() {
	clear_for_step();

	memset(key_down_states, 0, sizeof(key_down_states));

	keymod = KMOD_NONE;

	mouse_leftdown = false;
	mouse_rightdown = false;
	mouse_middledown = false;
}

void IOState::clear_for_step() {
	memset(key_press_states, 0, sizeof(key_press_states));

	active_events.clear();

	mouse_leftclick = false;
	mouse_rightclick = false;
	mouse_middleclick = false;

	mouse_leftrelease = false;
	mouse_rightrelease = false;

	mouse_didupwheel = false;
	mouse_diddownwheel = false;

}

static void bind_key_events(std::vector<IOEventTrigger>& bindings,
		const char* keys, IOEvent::event_t ev, SDLMod required, SDLMod rejected,
		bool allow_heldkey) {
	for (int i = 0; keys[i] != '\0'; i++) {
		IOEvent event(ev, i);
		IOEventTrigger trigger(event, IOEventTrigger::NONE, SDLKey(keys[i]),
				required, rejected, allow_heldkey);
		bindings.push_back(trigger);
	}
}
IOController::IOController() {
	//XXX: For now we hardcode the specific actions
	SDLMod item_mod = SDLMod(KMOD_LSHIFT | KMOD_RSHIFT);
	char action_key = 'j';

	// Spell choice
	bind_key_events(event_bindings, "yuiop", IOEvent::ACTIVATE_SPELL_N,
			KMOD_NONE, item_mod/*rejected*/, true);
//	bind_key_events(event_bindings, "12345", IOEvent::ACTIVATE_SPELL_N,
//			KMOD_NONE, item_mod/*rejected*/, true);

//	// Item choice
//	bind_key_events(event_bindings, "yuiop67890", IOEvent::USE_ITEM_N, item_mod,
//			KMOD_NONE, false);
	bind_key_events(event_bindings, "1234567890", IOEvent::USE_ITEM_N,
			KMOD_NONE, KMOD_NONE, false);

	/*Scroll spell up*/
	{
		IOEvent event(IOEvent::TOGGLE_ACTION_UP);
		IOEventTrigger trigger1(event, IOEventTrigger::MOUSE_WHEEL_UP,
				SDLKey(0), KMOD_NONE, KMOD_NONE, true);
		IOEventTrigger trigger2(event, IOEventTrigger::NONE, SDLKey('e'),
				KMOD_NONE, KMOD_NONE);
		IOEventTrigger trigger3(event, IOEventTrigger::NONE, SDLKey('.'),
				KMOD_NONE, KMOD_NONE);
		event_bindings.push_back(trigger1);
		event_bindings.push_back(trigger2);
		event_bindings.push_back(trigger3);
	}
	/*Scroll spell down*/
	{
		IOEvent event(IOEvent::TOGGLE_ACTION_DOWN);
		IOEventTrigger trigger1(event, IOEventTrigger::MOUSE_WHEEL_DOWN,
				SDLKey(0), KMOD_NONE, KMOD_NONE, true);
		IOEventTrigger trigger2(event, IOEventTrigger::NONE, SDLKey('q'),
				KMOD_NONE, KMOD_NONE);
		IOEventTrigger trigger3(event, IOEventTrigger::NONE, SDLKey(','),
				KMOD_NONE, KMOD_NONE);
		event_bindings.push_back(trigger1);
		event_bindings.push_back(trigger2);
		event_bindings.push_back(trigger3);
	}
	/*Use weapon*/
	{
		IOEvent event(IOEvent::USE_WEAPON);
		IOEventTrigger trigger1(event, IOEventTrigger::MOUSE_MIDDLE_CLICK,
				SDLKey(0), KMOD_NONE, KMOD_NONE, true);
		IOEventTrigger trigger2(event, IOEventTrigger::NONE, SDLKey('h'),
				KMOD_NONE, KMOD_NONE, true);
		event_bindings.push_back(trigger1);
		event_bindings.push_back(trigger2);
	}

	/*Do autotarget action */
	{
		IOEvent event(IOEvent::AUTOTARGET_CURRENT_ACTION);
		IOEventTrigger trigger1(event, IOEventTrigger::NONE, SDLKey(action_key),
				KMOD_NONE, KMOD_NONE, true);
		IOEventTrigger trigger2(event, IOEventTrigger::NONE, SDLK_SPACE,
				KMOD_NONE, KMOD_NONE, true);
		event_bindings.push_back(trigger1);
		event_bindings.push_back(trigger2);
	}

	/*Do targetted action */{
		IOEvent event(IOEvent::MOUSETARGET_CURRENT_ACTION);
		IOEventTrigger trigger(event, IOEventTrigger::MOUSE_LEFT_CLICK,
				SDLKey(0), KMOD_NONE, KMOD_NONE, true);
		event_bindings.push_back(trigger);
	}

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

int IOController::handle_event(SDL_Event* event) {
	int done = 0;
	iostate.keymod = event->key.keysym.mod;
	SDLKey key = event->key.keysym.sym;

	switch (event->type) {
	case SDL_ACTIVEEVENT:
		break;

	case SDL_KEYDOWN: {
		if (key == SDLK_ESCAPE) {
			done = 1;
		}
		iostate.key_down_states[key] = 1;
		iostate.key_press_states[key] = 1;
		break;
	}
	case SDL_KEYUP: {
		iostate.key_down_states[key] = 0;
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
		} else if (event->button.button == SDL_BUTTON_MIDDLE) {
			iostate.mouse_middledown = true;
			iostate.mouse_middleclick = true;
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
		} else if (event->button.button == SDL_BUTTON_MIDDLE) {
			iostate.mouse_middledown = false;
			iostate.mouse_middleclick = false;
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
void IOController::__trigger_events(IOEventTrigger::trigger_t trigger,
		SDLKey trigger_key, SDLMod mod, bool holding_key) {
	for (int i = 0; i < event_bindings.size(); i++) {
		IOEventTrigger& event_trigger = event_bindings[i];
		if (event_trigger.should_trigger(trigger, trigger_key, mod,
				holding_key)) {
			iostate.add_triggered_event(event_trigger.event, holding_key);
		}
	}
}

void IOController::trigger_events(const BBox& playing_area) {

	// Loop over all possible keys to find active key states
	for (int i = SDLK_FIRST + 1; i < SDLK_LAST; i++) {
		IOEventTrigger::trigger_t notrigger = IOEventTrigger::NONE;
		if (iostate.key_press_states[i]) {
			// Trigger key press events
			__trigger_events(notrigger, SDLKey(i), iostate.keymod, false);
		} else if (iostate.key_down_states[i]) {
			// Trigger key hold events
			__trigger_events(notrigger, SDLKey(i), iostate.keymod, true);
		}
	}
	if (playing_area.contains(mouse_x(), mouse_y())) {
		if (mouse_left_click() || mouse_left_down()) {
			__trigger_events(IOEventTrigger::MOUSE_LEFT_CLICK, SDLKey(0),
					iostate.keymod, !mouse_left_click());
		}
		if (mouse_right_click() || mouse_right_down()) {
			__trigger_events(IOEventTrigger::MOUSE_RIGHT_CLICK, SDLKey(0),
					iostate.keymod, !mouse_left_click());
		}
	}
	if (iostate.mouse_middleclick || iostate.mouse_middledown) {
		__trigger_events(IOEventTrigger::MOUSE_MIDDLE_CLICK, SDLKey(0),
				iostate.keymod, !iostate.mouse_middleclick);
	}
	if (mouse_upwheel()) {
		__trigger_events(IOEventTrigger::MOUSE_WHEEL_UP, SDLKey(0),
				iostate.keymod, false);
	}
	if (mouse_downwheel()) {
		__trigger_events(IOEventTrigger::MOUSE_WHEEL_DOWN, SDLKey(0),
				iostate.keymod, false);
	}
	if (mouse_downwheel()) {
		__trigger_events(IOEventTrigger::MOUSE_WHEEL_DOWN, SDLKey(0),
				iostate.keymod, false);
	}
}

bool IOController::query_event(const IOEvent& event, bool* triggered_already) {
	for (int i = 0; i < iostate.active_events.size(); i++) {
		if (event == iostate.active_events[i].event) {
			if (triggered_already) {
				*triggered_already = iostate.active_events[i].triggered_already;
			}
			return true;
		}
	}
	return false;
}

bool IOController::query_event(IOEvent::event_t event,
		bool* triggered_already) {
	for (int i = 0; i < iostate.active_events.size(); i++) {
		if (event == iostate.active_events[i].event.event_type) {
			if (triggered_already) {
				*triggered_already = iostate.active_events[i].triggered_already;
			}
			return true;
		}
	}
	return false;
}

