/*
 * IOController.h:
 *  Handles dispatch of events bound to keyboard keys
 */

#ifndef IOCONTROLLER_H_
#define IOCONTROLLER_H_

#include <vector>
#include <SDL/SDL.h>

#include "../../util/game_basic_structs.h"

// Multiple keys can refer to same action, in which case multiple
// GameEventTrigger instances can exist
struct GameEvent {
	/* Events with _N at the end require a number for disambiguation */
	enum event_t {
		USE_ITEM_N,
		AUTOTARGET_CURRENT_ACTION,
		SWITCH_TO_SPELL_N,
		SWITCH_TO_WEAPON,
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

	explicit GameEvent(event_t event_type, int event_num = 0) :
			event_type(event_type), event_num(event_num) {
	}
	bool operator==(const GameEvent& o) const {
		return event_type == o.event_type && event_num == o.event_num;
	}
};

// Encapsulates trigger + necessary action to emit
struct GameEventTrigger {
	enum trigger_t {
		MOUSE_LEFT_CLICK,
		MOUSE_LEFT_HOLD,
		MOUSE_RIGHT_CLICK,
		MOUSE_RIGHT_HOLD,
		MOUSE_WHEEL_UP,
		MOUSE_WHEEL_DOWN,
		NOT_MOVING,
		STOP_MOVING,
	};

	GameEvent event;
	trigger_t trigger;
	SDLKey trigger_key;
	SDLMod mod_required;
};

struct IOState {
	/* Keyboard states */
	char key_down_states[SDLK_LAST];
	char key_press_states[SDLK_LAST];

	/* Mouse position */
	int mousex, mousey;

	/* Mouse click states */
	bool mouse_leftdown, mouse_rightdown;
	bool mouse_leftclick, mouse_rightclick;
	bool mouse_leftrelease, mouse_rightrelease;
	bool mouse_didupwheel, mouse_diddownwheel;

	/* Action states */
	std::vector<GameEvent> active_events;

	IOState();
	void clear();
	void clear_for_step();
};

class IOController {
public:
	IOController();

	void bind_spell(spell_id spell);
	void unbind_spell(spell_id spell);

	void bind_item(spell_id spell);
	void unbind_item(spell_id spell);
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

	int key_down_state(int keyval);
	int key_press_state(int keyval);

	void update_iostate(bool resetprev);

	int handle_event(SDL_Event* event);
	bool query_event(const GameEvent& event);

private:
	IOState iostate;
	std::vector<spell_id> spells_bound;
	std::vector<item_id> items_bound;
	std::vector<GameEvent> event_bindings;
};

#endif /* IOCONTROLLER_H_ */
