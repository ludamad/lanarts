#include "GameScreen.h"
#include "GameState.h"
#include "objects/PlayerInst.h"

GameScreen& GameScreenSet::screen() {
    return screens.at(current_screen);
}
PlayerInst* GameScreenSet::focus_object(GameState* gs) {
    return gs->player_data().get(screen().focus_player_id).player();
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
