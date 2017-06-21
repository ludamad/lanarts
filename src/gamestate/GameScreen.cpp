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

static const int GAME_SIDEBAR_WIDTH = 160; // TODO consolidate this with GameState.cpp
void GameScreenSet::deserialize(GameState *gs, SerializeBuffer &serializer) {
    serializer.read_int(current_screen);
    simulation_map = read_gamemap(gs, serializer);
    serializer.read_container(screens, [&](GameScreen& screen) {
        screen.deserialize(gs, serializer);
    });

    GameSettings& settings = gs->game_settings();
    int n_local_players = 0;
    for (PlayerDataEntry &player: gs->player_data().all_players()) {
        if (player.is_local_player) {
            n_local_players++;
        }
    }
    // Number of split-screens tiled together
    int n_x = 1, n_y = 1;
    if (n_local_players <= 2) {
        n_x = n_local_players;
    } else if (n_local_players <= 4) {
        // More than 2, less than 4? Try 2x2 tiling
        n_x = 2, n_y = 2;
    } else if (n_local_players <= 6) {
        n_x = 3, n_y = 2;
    } else {
        LANARTS_ASSERT(n_local_players <= 9);
        // Last resort, Try 3x3 tiling
        n_x = 3, n_y = 3;
    }

    const int WIDTH = settings.view_width / n_x;
    const int HEIGHT = settings.view_height / n_y; // / N_PLAYERS;
    std::vector<BBox> bounding_boxes;
    for (GameScreen& screen : screens) {
        const int x1 = (screen.index % n_x) * WIDTH, y1 = (screen.index / n_x) * HEIGHT;
        bounding_boxes.push_back(BBox {x1, y1, x1 + WIDTH, y1 + HEIGHT});
    }
    if (bounding_boxes.size() == 3) {
        bounding_boxes[1] = {WIDTH, 0, settings.view_width, settings.view_height};
    }
    for (GameScreen& screen : screens) {
        BBox b = bounding_boxes[screen.index];
        screen.window_region = b;
        screen.hud = GameHud {
                BBox(b.x2 - GAME_SIDEBAR_WIDTH, b.y1, b.x2, b.y2),
                BBox(b.x1, b.y1, b.x2 - GAME_SIDEBAR_WIDTH, b.y2)
        };
        screen.view = GameView {0, 0, b.width() - GAME_SIDEBAR_WIDTH, b.height()};
    }
}

void GameScreen::serialize(GameState *gs, SerializeBuffer &serializer) const {
    serializer.write_int(index);
    // hud details not serialized as they are ephermal user interface state
    serializer.write(focus_player_id);
    write_gamemap(gs, serializer, current_level);
    serializer.write(last_player_pos);
}
void GameScreen::deserialize(GameState *gs, SerializeBuffer &serializer) {
    serializer.read_int(index);
    // hud details not serialized as they are ephermal user interface state
    serializer.read(focus_player_id);
    current_level = read_gamemap(gs, serializer);
    serializer.read(last_player_pos);
}
