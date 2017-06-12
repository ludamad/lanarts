/*
 * lua_core_Gamepad.cpp:
 *  Bindings for gamepad state queries
 */
 
#include <luawrap/luawrap.h>

#include "gamestate/GameState.h"
#include "gamestate/IOController.h"

#include "lua_api.h"

static void trigger_action(LuaStackValue action) {
    GameState* gs = lua_api::gamestate(action.luastate());
    int action_type = action.as<int>();

    IOEvent event {(IOEvent::event_t)action_type};
    gs->io_controller().push_event(event);
}

static void trigger_analog_action(LuaStackValue action, LuaStackValue magnitude) {
    GameState* gs = lua_api::gamestate(action.luastate());
    int action_type = action.as<int>();
    float analog_magnitude = magnitude.as<float>();

    IOEvent event {(IOEvent::event_t)action_type, 0, analog_magnitude};
    gs->io_controller().push_event(event);
}

namespace lua_api {
    void register_lua_core_GameActions(lua_State* L) {
        LuaValue actions = lua_api::register_lua_submodule(L, "core.GameActions");
        actions["trigger_action"].bind_function(trigger_action);
        actions["trigger_analog_action"].bind_function(trigger_analog_action);
        actions["USE_ITEM_N"] = (int)IOEvent::event_t::USE_ITEM_N;
        actions["SELL_ITEM_N"] = (int)IOEvent::event_t::SELL_ITEM_N;
        actions["AUTOTARGET_CURRENT_ACTION"] = (int)IOEvent::event_t::AUTOTARGET_CURRENT_ACTION;
        actions["ACTIVATE_SPELL_N"] = (int)IOEvent::event_t::ACTIVATE_SPELL_N;
        actions["USE_WEAPON"] = (int)IOEvent::event_t::USE_WEAPON;
        actions["TOGGLE_ACTION_UP"] = (int)IOEvent::event_t::TOGGLE_ACTION_UP;
        actions["TOGGLE_ACTION_DOWN"] = (int)IOEvent::event_t::TOGGLE_ACTION_DOWN;
        actions["MOUSETARGET_CURRENT_ACTION"] = (int)IOEvent::event_t::MOUSETARGET_CURRENT_ACTION;
        actions["MOUSETARGET_MOVE_TOWARDS"] = (int)IOEvent::event_t::MOUSETARGET_MOVE_TOWARDS;
        actions["MOVE_X_M"] = (int)IOEvent::event_t ::MOVE_X_M;
        actions["MOVE_Y_M"] = (int)IOEvent::event_t ::MOVE_Y_M;
        actions["EXIT_GAME"] = (int)IOEvent::event_t::EXIT_GAME;
    }
}
