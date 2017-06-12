/*
 * IOController.h:
 *  Handles polling for IO state.
 *  Used to define core.Mouse, core.Keyboard and core.Gamepad Lua modules.
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

	std::vector<SDL_Event> sdl_events;
	std::vector<IOGamepadState> gamepad_states;
    
	IOState();
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

	bool user_has_exit() const;
	bool user_has_requested_exit();
	void clear();

private:
	IOState iostate;

    IOGamepadState create_gamepad_state(SDL_GameController* controller, int id);
    
	std::vector<spell_id> spells_bound;
	std::vector<item_id> items_bound;
    std::vector<SDL_GameController*> controllers;

    void reinit_controllers();
};

#endif /* IOCONTROLLER_H_ */
