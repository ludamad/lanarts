/*
 * IOController.h:
 *  Handles dispatch of events bound to keyboard keys
 */

#ifndef IOCONTROLLER_H_
#define IOCONTROLLER_H_

#include <vector>
#include <unordered_map>
#include <SDL.h>
#include <SDL_gamecontroller.h>
#include <SDL_joystick.h>

#include "lanarts_defines.h"

struct BBox;

// Multiple keys can refer to same action, in which case multiple
// GameEventTrigger instances can exist
struct IOEvent {
	/* Events with _N at the end require a number for disambiguation */
    /* Events with _M at the end require a magnitude */
	enum event_t {
        USE_ITEM_N,
        SELL_ITEM_N,
		AUTOTARGET_CURRENT_ACTION,
		ACTIVATE_SPELL_N,
		USE_WEAPON,
		TOGGLE_ACTION_UP,
		TOGGLE_ACTION_DOWN,
		// Rely on mouse coordinates:
		MOUSETARGET_CURRENT_ACTION,
		MOUSETARGET_MOVE_TOWARDS,
        // Rely on magnitude
        MOVE_X_M,
        MOVE_Y_M,
		// Non-gameplay actions:
		EXIT_GAME
	};
	event_t event_type;
	int event_num; // Not-always-necessary event data
    float event_magnitude; //Not-always-necessary event magnitude

	explicit IOEvent(event_t event_type, int event_num = 0, float event_magnitude = 0.0) :
			event_type(event_type), event_num(event_num), event_magnitude(event_magnitude) {
	}
	bool operator==(const IOEvent& o) const {
		return event_type == o.event_type && event_num == o.event_num;
	}
};

// Encapsulates trigger + necessary action to emit
struct IOEventTrigger {
	enum trigger_t {
		NONE = 0,
		MOUSE_LEFT_CLICK,
		MOUSE_RIGHT_CLICK,
		MOUSE_WHEEL_UP,
		MOUSE_WHEEL_DOWN,
		MOUSE_MIDDLE_CLICK,
		NOT_MOVING,
		STOP_MOVING
	};

	IOEvent event;
	trigger_t trigger;
	SDL_Keycode trigger_key;
	SDL_Keymod mod_required;
	SDL_Keymod mod_rejected;
	// Does holding the key always trigger this event ?
	bool trigger_continuously;

	bool should_trigger(IOEventTrigger::trigger_t trigger, SDL_Keycode trigger_key,
			SDL_Keymod mod, bool holding_key);

	IOEventTrigger(const IOEvent& event, trigger_t trigger = NONE,
			SDL_Keycode trigger_key = SDLK_UNKNOWN, SDL_Keymod mod_required = KMOD_NONE,
			SDL_Keymod mod_rejected = KMOD_NONE, bool trigger_continuously = false) :
			event(event), trigger(trigger), trigger_key(trigger_key), mod_required(
					mod_required), mod_rejected(mod_rejected), trigger_continuously(
					trigger_continuously) {

	}
};

struct IOGamepadState {
    int gamepad_id;
    float gamepad_axis_left_trigger;
    float gamepad_axis_right_trigger;
    float gamepad_axis_left_x;
    float gamepad_axis_left_y;
    float gamepad_axis_right_x;
    float gamepad_axis_right_y;
    bool gamepad_button_a;
    bool gamepad_button_b;
    bool gamepad_button_x;
    bool gamepad_button_y;
    bool gamepad_button_back;
    bool gamepad_button_guide;
    bool gamepad_button_start;
    bool gamepad_button_left_stick;
    bool gamepad_button_right_stick;
    bool gamepad_button_left_shoulder;
    bool gamepad_button_right_shoulder;
    bool gamepad_button_up_dpad;
    bool gamepad_button_down_dpad;
    bool gamepad_button_left_dpad;
    bool gamepad_button_right_dpad;
};

struct IOState {
	struct TriggeredIOEvent {
		IOEvent event;
		bool triggered_already;
		TriggeredIOEvent(const IOEvent& event, bool triggered_already) :
				event(event), triggered_already(triggered_already) {
		}
	};

    
    std::unordered_map<SDL_Keycode, bool> key_down_states;
    std::unordered_map<SDL_Keycode, bool> key_press_states;
	SDL_Keymod keymod;
	// Mouse io state
	int mousex, mousey;
	bool mouse_leftdown, mouse_rightdown;
	bool mouse_leftclick, mouse_rightclick;
	bool mouse_middleclick, mouse_middledown;
	bool mouse_leftrelease, mouse_rightrelease;
	bool mouse_didupwheel, mouse_diddownwheel;

	std::vector<TriggeredIOEvent> active_events;
	std::vector<SDL_Event> sdl_events;
	std::vector<IOGamepadState> gamepad_states;
    
	IOState();
	void add_triggered_event(const IOEvent & event, bool triggered_already);
	void clear();
	void clear_for_step(bool resetprev = true);
};

class IOController {
public:
	IOController();

	void bind_spell(spell_id spell);
	void unbind_spell(spell_id spell);

	void bind_item(spell_id spell);
	void unbind_item(spell_id spell);
	bool ctrl_held();
    bool shift_held();
	bool mouse_left_click();
	bool mouse_right_click();
	bool mouse_left_down();
	bool mouse_right_down();
	bool mouse_left_release();
	bool mouse_right_release();
	bool mouse_upwheel();
	bool mouse_downwheel();

	int mouse_x();
	int mouse_y();

	std::vector<IOGamepadState>& gamepad_states();

	bool key_down_state(int keyval);
	bool key_press_state(int keyval);

    void set_key_down_state(int keyval);
    void set_key_press_state(int keyval);

	std::vector<SDL_Event>& get_events();

	void update_iostate(bool resetprev);
	void trigger_events(const BBox& playing_area);

	int handle_event(SDL_Event* event);
    void push_event(const IOEvent& event);
	bool query_event(const IOEvent& event, bool* triggered_already = NULL);
	bool query_event(IOEvent::event_t event, bool* triggered_already = NULL);

	bool user_has_exit() const;
	bool user_has_requested_exit();
	void clear();

private:
	void __trigger_events(IOEventTrigger::trigger_t trigger, SDL_Keycode trigger_key,
			SDL_Keymod mod, bool holding_key);
	IOState iostate;

    IOGamepadState create_gamepad_state(SDL_GameController* controller, int id);
    
	std::vector<spell_id> spells_bound;
	std::vector<item_id> items_bound;
	std::vector<IOEventTrigger> event_bindings;
    std::vector<SDL_GameController*> controllers;

    void reinit_controllers();
};

#endif /* IOCONTROLLER_H_ */
