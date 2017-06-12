#include "GameScreen.h"
#include "GameState.h"
#include "objects/PlayerInst.h"

GameScreen& GameScreenSet::screen() {
    return screens.at(current_screen);
}
PlayerInst* GameScreenSet::focus_object(GameState* gs) {
    return local_player_data(gs).player();
}
PlayerDataEntry& GameScreenSet::local_player_data(GameState* gs) {
    if (current_screen == -1) {
        return gs->player_data().get(screens.at(0).focus_player_id);
    } else {
        return gs->player_data().get(screen().focus_player_id);
    }
}
GameMapState* GameScreenSet::map(GameState* gs) {
    return focus_object(gs)->get_map(gs);
}
GameHud& GameScreenSet::hud() {
    return screen().hud;
}
GameView& GameScreenSet::view() {
    return screen().view;
}
BBox& GameScreenSet::window_region() {
    return screen().window_region;
}

GameScreen& GameScreenSet::get_screen(GameState* gs, PlayerInst* player) {
    int screen = -1;
    for_each_screen([&]() {
        if (focus_object(gs) == player) {
            screen = current_screen;
        }
    });
    return screens.at(screen);
}
