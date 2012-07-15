/*
 * GameState.h:
 *  Handle to all the global game data.
 */

#ifndef GAMESTATE_H_
#define GAMESTATE_H_

#include <vector>
#include <SDL.h>

#include "../display/font.h"

#include "../fov/fov.h"

#include "../util/mtwist.h"

#include "../net/GameNetConnection.h"

#include "controllers/IOController.h"
#include "controllers/MonsterController.h"
#include "controllers/PlayerController.h"

#include "ui/GameChat.h"
#include "ui/GameHud.h"
#include "ui/GameDialogs.h"

#include "GameSettings.h"
#include "GameView.h"
#include "GameWorld.h"

struct lua_State;
struct GameLevelState;
struct GameTiles;

class GameInst;
class PlayerInst;

class GameState {
public:

	GameState(const GameSettings& settings, lua_State* L, int vieww = 640,
			int viewh = 480, int hudw = 160);
	~GameState();
	/* Call after construction, before game starts: */
	void init_game();

	/* Primary events */
	void draw(bool drawhud = true);
	bool pre_step();
	void step();
	bool update_iostate(bool resetprev = true);

	void adjust_view_to_dragging();

	/* Instance retrieval and removal functions */
	GameInst* get_instance(obj_id id);
	obj_id add_instance(GameInst* inst);
	void remove_instance(GameInst* inst);
	//Skip an instance id as if we were making an instance
	//used for synchronization purposes in network play
	void skip_next_instance_id();

	obj_id local_playerid();
	PlayerInst* local_player();

	/* Dimensions (in pixels) of game world */
	int width();
	int height();

	/* Collision test functions */
	bool tile_radius_test(int x, int y, int rad, bool issolid = true,
			int ttype = -1, Pos* hitloc = NULL);
	bool tile_line_test(int x, int y, int w, int h);
	int object_radius_test(GameInst* obj, GameInst** objs = NULL, int obj_cap =
			0, col_filterf f = NULL, int x = -1, int y = -1, int radius = -1);
	int object_radius_test(int x, int y, int radius, col_filterf f = NULL,
			GameInst** objs = NULL, int obj_cap = 0);
	bool solid_test(GameInst* obj, GameInst** objs = NULL, int obj_cap = 0,
			col_filterf f = NULL, int x = -1, int y = -1, int radius = -1) {
		int lx = (x == -1 ? obj->x : x), ly = (y == -1 ? obj->y : y);
		return tile_radius_test(lx, ly, radius == -1 ? obj->radius : radius)
				|| object_radius_test(obj, objs, obj_cap, f, x, y, radius);
	}

	bool solid_test(GameInst* obj, int x, int y, int radius = -1) {
		return solid_test(obj, NULL, 0, NULL, x, y, radius);
	}

	/* player's field-of-view visibility tests */
	bool object_visible_test(GameInst* obj, PlayerInst* player = NULL,
			bool canreveal = true);
	/* player's field-of-view visibility tests */
	bool radius_visible_test(int x, int y, int radius, PlayerInst* player = NULL,
			bool canreveal = true);

	/* GameState components */
	GameView& window_view() {
		return view;
	}

	GameTiles& tile_grid();
	GameHud& game_hud() {
		return hud;
	}

	GameWorld& game_world() {
		return world;
	}

	GameTextConsole& game_console() {
		return game_hud().game_console();
	}

	GameChat& game_chat() {
		return game_hud().game_chat();
	}

	GameSettings& game_settings() {
		return settings;
	}

	GameNetConnection& net_connection() {
		return connection;
	}

	lua_State* get_luastate() {
		return L;
	}
	/* A single mersenne twister state is used for all random number generation */
	MTwist& rng() {
		return mtwist;
	}

	/* Instance global controllers */
	MonsterController& monster_controller();
	PlayerController& player_controller();
	EnemiesSeen& enemies_seen() {
		return world.enemies_seen();
	}

	/* Font getters */
	const font_data& primary_font() {
		return small_font;
	}

	const font_data& menu_font() {
		return large_font;
	}

	/* IO & action controller */
	IOController& io_controller() {
		return iocontroller;
	}

	/* Mouse position */
	int mouse_x() {
		return iocontroller.mouse_x();
	}

	int mouse_y() {
		return iocontroller.mouse_y();
	}

	/* Mouse click states */
	bool mouse_left_click();
	bool mouse_right_click();

	bool mouse_left_down();
	bool mouse_right_down();

	bool mouse_left_release();
	bool mouse_right_release();

	bool mouse_upwheel();
	bool mouse_downwheel();

	/* Current frame number */
	int& frame() {
		return frame_n;
	}

	/* Keyboard press states */
	int key_down_state(int keyval);
	int key_press_state(int keyval);

	/* Level utility functions */
	GameLevelState* get_level() {
		return world.get_current_level();
	}

	void set_level(GameLevelState* lvl);
	void level_move(int id, int x, int y, int roomid1, int roomid2);
	/* Make sure rooms exist & portals point to valid locations in next room */
	void ensure_level_connectivity(int roomid1, int roomid2);

	void serialize(FILE* file);

private:
	int handle_event(SDL_Event* event);

	/* Members */
	GameSettings settings;
	lua_State* L;
	int frame_n;
	GameNetConnection connection;
	GameDialogs dialogs;
	GameHud hud;
	GameView view;
	GameWorld world;
	MTwist mtwist;
	font_data small_font, large_font;
	bool dragging_view;
	IOController iocontroller;
};

#endif /* GAMESTATE_H_ */
