//
// Created by guest on 12/06/17.
//

#include <luawrap/luawrap.h>
#include "PlayerIOActions.h"

template <typename T>
static T method_call(LuaValue value, const char *method) {
    lua_State* L = value.luastate();
    value[method].push();
    // Since it is a method, use the object itself as the first (and only) argument:
    return luawrap::call<T>(L, value);
}

PosF PlayerIOActions::move_direction() {
    return method_call<PosF>(value, "move_direction");
}

bool PlayerIOActions::should_explore() {
    return method_call<bool>(value, "should_explore");
}

int PlayerIOActions::use_item_slot() {
    return method_call<int>(value, "use_item_slot");
}

int PlayerIOActions::sell_item_slot() {
    return method_call<int>(value, "sell_item_slot");
}

int PlayerIOActions::use_spell_slot() {
    return method_call<int>(value, "use_spell_slot");
}

PosF PlayerIOActions::target_position() {
    return method_call<PosF>(value, "target_position");
}

bool PlayerIOActions::should_use_weapon() {
    return method_call<bool>(value, "should_use_weapon");
}

bool PlayerIOActions::should_shift_autotarget() {
    return method_call<bool>(value, "should_shift_autotarget");
}

bool PlayerIOActions::should_exit_game() {
    return method_call<bool>(value, "should_exit_game");
}

void PlayerIOActions::poll_input() {
    method_call<void>(value, "poll_input");
}
