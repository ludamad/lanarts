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
    int gamepad_id = -1;
    float gamepad_axis_left_trigger = 0;
    float gamepad_axis_right_trigger = 0;
    float gamepad_axis_left_x = 0;
    float gamepad_axis_left_y = 0;
    float gamepad_axis_right_x = 0;
    float gamepad_axis_right_y = 0;
    bool gamepad_button_a = false;
    bool gamepad_button_b = false;
    bool gamepad_button_x = false;
    bool gamepad_button_y = false;
    bool gamepad_button_back = false;
    bool gamepad_button_guide = false;
    bool gamepad_button_start = false;
    bool gamepad_button_left_stick = false;
    bool gamepad_button_right_stick = false;
    bool gamepad_button_left_shoulder = false;
    bool gamepad_button_right_shoulder = false;
    bool gamepad_button_up_dpad = false;
    bool gamepad_button_down_dpad = false;
    bool gamepad_button_left_dpad = false;
    bool gamepad_button_right_dpad = false;
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
