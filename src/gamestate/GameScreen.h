#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <lua.hpp>
#include "lanarts_defines.h"
#include "GameView.h"

#include "interface/GameChat.h"
#include "interface/GameHud.h"

class GameInst;
class GameMapState;
class SerializeBuffer;

struct PlayerDataEntry;

struct GameScreen {
    int index;
    GameHud hud;
    GameView view;
    BBox window_region;
    int focus_player_id;
    GameMapState* current_level;
    Pos last_player_pos;
    void serialize(GameState* gs, SerializeBuffer& serializer) const;
    void deserialize(GameState* gs, SerializeBuffer& serializer);
};

// Represents all active screens being drawn
// Multiple screens are used in split screen, while no screens are used
// when lanarts is being run headlessly.
// GameScreen was introduced when implementing split-screen.
// A lot of code that was originally a single statement had to become a loop over all screens.
struct GameScreenSet {
    void add(GameScreen screen) {
        screen.index = (int)screens.size();
        screens.push_back(screen);
    }

    GameScreen& screen();
    GameView& view();
    PlayerInst* focus_object(GameState* gs);
    PlayerDataEntry& local_player_data(GameState* gs);
    GameMapState* map(GameState* gs);
    GameHud& hud();
    BBox& window_region();

    void clear() {
        screens.clear();
    }
    template <typename Func>
    void for_each_screen(Func&& f, bool allow_nesting = false) {
        // Do not allow nesting of for_each_screen;
        LANARTS_ASSERT(allow_nesting || current_screen == -1);
	int prev_screen = current_screen;
        for (current_screen = 0; current_screen < screens.size(); current_screen++) {
            f();
        }
        current_screen = prev_screen;
    };

    GameScreen& get_screen(GameState *gs, PlayerInst *player);
    void set_screen(int screen_idx) {
        current_screen = screen_idx;
    }
    size_t amount() {
        return screens.size();
    }

    GameMapState* get_current_level();
    void set_current_level(GameMapState *map);
    void serialize(GameState* gs, SerializeBuffer& serializer);
    void deserialize(GameState* gs, SerializeBuffer& serializer);

    int stash_screen() {
        int stash = current_screen;
        current_screen = -1;
        return stash;
    }
    void unstash_screen(int stash) {
        LANARTS_ASSERT(current_screen == -1);
        current_screen = stash;
    }
    int screen_index() {
        return current_screen;
    }
private:
    int current_screen = -1;
    // For non-drawing purposes:
    GameMapState* simulation_map = NULL;
    std::vector<GameScreen> screens;
};

#endif //GAMESCREEN_H
