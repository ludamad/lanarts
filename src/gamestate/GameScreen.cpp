#include <lcommon/SerializeBuffer.h>

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
        LANARTS_ASSERT(false);
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

void GameScreenSet::set_current_level(GameMapState* map) {
    if (current_screen == -1) {
        simulation_map = map;
        return;
    }
    screen().current_level = map;
    if (map != NULL) {
        screen().view.world_width = map->width();
        screen().view.world_height = map->height();
    }
}
GameMapState* GameScreenSet::get_current_level() {
    if (current_screen == -1) {
        return simulation_map;
    }
    return screen().current_level;
}

static GameMapState* read_gamemap(GameState *gs, SerializeBuffer &serializer) {
    level_id id = serializer.read_int();
    if (id == -1) {
        return nullptr;
    }
    return gs->get_level(id);
}

static void write_gamemap(GameState *gs, SerializeBuffer &serializer, GameMapState* map) {
    if (map == NULL) {
        serializer.write_int(-1);
        return;
    }
    serializer.write_int(map->id());
}

void GameScreenSet::serialize(GameState *gs, SerializeBuffer &serializer) {

//    int current_screen = -1;
//    // For non-drawing purposes:
//    GameMapState* simulation_map = NULL;
//    std::vector<GameScreen> screens;
    serializer.write_int(current_screen);
    write_gamemap(gs, serializer, simulation_map);
    serializer.write_container(screens, [&](const GameScreen& screen) {
        screen.serialize(gs, serializer);
    });
}

void GameScreenSet::deserialize(GameState *gs, SerializeBuffer &serializer) {
    serializer.read_int(current_screen);
    simulation_map = read_gamemap(gs, serializer);
    serializer.read_container(screens, [&](GameScreen& screen) {
        screen.deserialize(gs, serializer);
    });
}

void GameScreen::serialize(GameState *gs, SerializeBuffer &serializer) const {
    serializer.write_int(index);
    // hud not serialized as its ephermal user interface state
    serializer.write(view);
    serializer.write(window_region);
    serializer.write(focus_player_id);
    write_gamemap(gs, serializer, current_level);
    serializer.write(last_player_pos);
}
static const int GAME_SIDEBAR_WIDTH = 160; // TODO consolidate this with GameState.cpp
void GameScreen::deserialize(GameState *gs, SerializeBuffer &serializer) {
    serializer.read_int(index);
    // hud not serialized as its ephermal user interface state
    serializer.read(view);
    serializer.read(window_region);
    serializer.read(focus_player_id);
    current_level = read_gamemap(gs, serializer);
    serializer.read(last_player_pos);

    BBox b = window_region;
    hud = GameHud {
            BBox(b.x2 - GAME_SIDEBAR_WIDTH, b.y1, b.x2, b.y2),
            BBox(b.x1, b.y1, b.x2 - GAME_SIDEBAR_WIDTH, b.y2)
    };
}