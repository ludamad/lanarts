/*
 * lua_core_Gamepad.cpp:
 *  Bindings for gamepad state queries
 */
 
#include <luawrap/luawrap.h>

#include "gamestate/GameState.h"

#include "lua_api.h"
 
 
 
 
static IOGamepadState* gamepad_by_id(LuaStackValue lgamepad_id){
    int gamepad_id = lgamepad_id.as<int>();
        GameState* gs = lua_api::gamestate(lgamepad_id.luastate());
        for (auto& gamepad : gs->gamepad_states()) {
            if(gamepad_id == gamepad.gamepad_id){
                return &gamepad;
            }
        }
        return NULL;
}

#define GAMEPAD_BUTTON_GETTER(member) \
 static bool member (LuaStackValue lgamepad_id) { \
     auto *gamepad = gamepad_by_id(lgamepad_id); \
     if(gamepad == NULL){ \
         return false; \
     } \
     return gamepad-> member; \
 }

#define GAMEPAD_AXIS_GETTER(member) \
static float member (LuaStackValue lgamepad_id) { \
    auto *gamepad = gamepad_by_id(lgamepad_id); \
    if(gamepad == NULL){ \
        return false; \
    } \
    return gamepad-> member; \
}

GAMEPAD_AXIS_GETTER( gamepad_axis_left_trigger );
GAMEPAD_AXIS_GETTER( gamepad_axis_right_trigger );
GAMEPAD_AXIS_GETTER( gamepad_axis_left_x );
GAMEPAD_AXIS_GETTER( gamepad_axis_left_y );
GAMEPAD_AXIS_GETTER( gamepad_axis_right_x );
GAMEPAD_AXIS_GETTER( gamepad_axis_right_y );
 
GAMEPAD_BUTTON_GETTER( gamepad_button_a );
GAMEPAD_BUTTON_GETTER( gamepad_button_b );
GAMEPAD_BUTTON_GETTER( gamepad_button_x );
GAMEPAD_BUTTON_GETTER( gamepad_button_y );
GAMEPAD_BUTTON_GETTER( gamepad_button_back );
GAMEPAD_BUTTON_GETTER( gamepad_button_guide );
GAMEPAD_BUTTON_GETTER( gamepad_button_start );
GAMEPAD_BUTTON_GETTER( gamepad_button_left_stick );
GAMEPAD_BUTTON_GETTER( gamepad_button_right_stick );
GAMEPAD_BUTTON_GETTER( gamepad_button_left_shoulder );
GAMEPAD_BUTTON_GETTER( gamepad_button_right_shoulder );
GAMEPAD_BUTTON_GETTER( gamepad_button_up_dpad );
GAMEPAD_BUTTON_GETTER( gamepad_button_down_dpad );
GAMEPAD_BUTTON_GETTER( gamepad_button_left_dpad );
GAMEPAD_BUTTON_GETTER( gamepad_button_right_dpad );
 
namespace lua_api {
    void register_lua_core_Gamepad(lua_State* L) {
        LuaValue gamepad = lua_api::register_lua_submodule(L, "core.Gamepad");
        gamepad["gamepad_axis_left_trigger"].bind_function(gamepad_axis_left_trigger);
        gamepad["gamepad_axis_right_trigger"].bind_function(gamepad_axis_right_trigger);
        gamepad["gamepad_axis_left_x"].bind_function(gamepad_axis_left_x);
        gamepad["gamepad_axis_left_y"].bind_function(gamepad_axis_left_y);
        gamepad["gamepad_axis_right_x"].bind_function(gamepad_axis_right_x);
        gamepad["gamepad_axis_right_y"].bind_function(gamepad_axis_right_y);
        
        gamepad["gamepad_button_a"].bind_function(gamepad_button_a);
        gamepad["gamepad_button_b"].bind_function(gamepad_button_b);
        gamepad["gamepad_button_x"].bind_function(gamepad_button_x);
        gamepad["gamepad_button_y"].bind_function(gamepad_button_y);
        gamepad["gamepad_button_back"].bind_function(gamepad_button_back);
        gamepad["gamepad_button_guide"].bind_function(gamepad_button_guide);
        gamepad["gamepad_button_start"].bind_function(gamepad_button_start);
        gamepad["gamepad_button_left_stick"].bind_function(gamepad_button_left_stick);
        gamepad["gamepad_button_right_stick"].bind_function(gamepad_button_right_stick);
        gamepad["gamepad_button_left_shoulder"].bind_function(gamepad_button_left_shoulder);
        gamepad["gamepad_button_right_shoulder"].bind_function(gamepad_button_right_shoulder);
        gamepad["gamepad_button_up_dpad"].bind_function(gamepad_button_up_dpad);
        gamepad["gamepad_button_down_dpad"].bind_function(gamepad_button_down_dpad);
        gamepad["gamepad_button_left_dpad"].bind_function(gamepad_button_left_dpad);
        gamepad["gamepad_button_right_dpad"].bind_function(gamepad_button_right_dpad);
    }
}
