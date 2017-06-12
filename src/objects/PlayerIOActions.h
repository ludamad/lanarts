#ifndef PLAYERIOACTIONS_H
#define PLAYERIOACTIONS_H

#include <lcommon/geometry.h>
#include <luawrap/LuaValue.h>
#include <lanarts_defines.h>

class PlayerIOActions {
public:
    PosF move_direction();
    bool should_explore();
    int use_item_slot(); // -1 if none
    int sell_item_slot(); // -1 if none
    int use_spell_slot(); // -1 if none
    PosF target_position(); // Pos(0,0) if none
    bool should_use_weapon();
    bool should_exit_game();
    void poll_input();
    void init(LuaValue value) {
        this->value = value;
    }
    LuaValue value;

    bool should_shift_autotarget();
};

#endif //PLAYERIOACTIONS_H
