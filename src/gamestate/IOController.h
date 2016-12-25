/*
 * IOController.h:
 *  Handles dispatch of events bound to keyboard keys
 */

#ifndef IOCONTROLLER_H_
#define IOCONTROLLER_H_

#include <vector>
#include <unordered_map>
#include <SDL.h>

#include "lanarts_defines.h"

struct BBox;

// Multiple keys can refer to same action, in which case multiple
// GameEventTrigger instances can exist
struct IOEvent {
	/* Events with _N at the end require a number for disambiguation */
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
		// Non-gameplay actions:
		EXIT_GAME
	};
	event_t event_type;
	int event_num; // Not-always-necessary event data

	explicit IOEvent(event_t event_type, int event_num = 0) :
			event_type(event_type), event_num(event_num) {
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
	int mousex, mousey;
	bool mouse_leftdown, mouse_rightdown;
	bool mouse_leftclick, mouse_rightclick;
	bool mouse_middleclick, mouse_middledown;
	bool mouse_leftrelease, mouse_rightrelease;
	bool mouse_didupwheel, mouse_diddownwheel;
	std::vector<TriggeredIOEvent> active_events;
	std::vector<SDL_Event> sdl_events;

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

        bool key_down_state(int keyval);
	bool key_press_state(int keyval);

	std::vector<SDL_Event>& get_events();

	void update_iostate(bool resetprev);
	void trigger_events(const BBox& playing_area);

	int handle_event(SDL_Event* event);
	bool query_event(const IOEvent& event, bool* triggered_already = NULL);
	bool query_event(IOEvent::event_t event, bool* triggered_already = NULL);

	bool user_has_exit() const;

private:
	void __trigger_events(IOEventTrigger::trigger_t trigger, SDL_Keycode trigger_key,
			SDL_Keymod mod, bool holding_key);
	IOState iostate;

	std::vector<spell_id> spells_bound;
	std::vector<item_id> items_bound;
	std::vector<IOEventTrigger> event_bindings;
};

#endif /* IOCONTROLLER_H_ */
