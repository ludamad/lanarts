/*
 * GameState.h:
 *  Handle to all the global game data. Note this is somewhat of a 'god class'.
 *  This is tolerated mainly as it simply forwards a lot of calls to various components,
 *  without making the caller worry about which component does what.
 */

#ifndef GAMESTATE_H_
#define GAMESTATE_H_

#include <vector>
#include <SDL.h>
#include <cstdio>

#include "../display/font.h"

#include "../fov/fov.h"

#include "../interface/GameChat.h"

#include "../interface/GameHud.h"
#include "../net/GameNetConnection.h"

#include "../util/mtwist.h"
#include "GameLogger.h"
#include "GameSettings.h"
#include "GameView.h"

#include "GameWorld.h"
#include "IOController.h"
#include "PlayerData.h"

struct lua_State;
class GameLevelState;
struct GameTiles;

class GameInst;
class PlayerInst;
class CollisionAvoidance;
class MonsterController;
class Serializer;

struct GameStateInitData {
	int seed;
	bool seed_set_by_network_message;
	GameStateInitData(int seed = 0, bool seed_set_by_network_message = false) :
			seed(seed), seed_set_by_network_message(seed_set_by_network_message) {
	}
};

class GameState {
public:

	GameState(const GameSettings& settings, lua_State* L, int vieww = 640,
			int viewh = 480, int hudw = 160);
	~GameState();
	void start_connection();
	/* Call after construction, before game starts: */
	void start_game();
	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

	/* Primary events */
	void draw(bool drawhud = true);
	bool pre_step();
	void step();
	bool update_iostate(bool resetprev = true);

	void adjust_view_to_dragging();

	/* Instance retrieval and removal functions */
	GameInst* get_instance(obj_id id);
	GameInst* get_instance(level_id level, obj_id id);
	obj_id add_instance(GameInst* inst);
	obj_id add_instance(level_id level, GameInst* inst);
	void remove_instance(GameInst* inst);
	//Skip an instance id as if we were making an instance
	//used for synchronization purposes in network play
	void skip_next_instance_id();

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
	bool radius_visible_test(int x, int y, int radius,
			PlayerInst* player = NULL, bool canreveal = true);

	/* GameState components */
	GameView& view() {
		return _view;
	}

	GameTiles& tiles();
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
	std::vector<PlayerInst*> players_in_level();
	bool level_has_player();

	PlayerData& player_data() {
		return world.player_data();
	}

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

	Pos mouse_pos() {
		return Pos(mouse_x(), mouse_y());
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

	// Use this to override seed used in start_game()
	GameStateInitData& game_state_init_data() {
		return init_data;
	}

	void set_repeat_actions_counter(int repeat_actions_counter) {
		this->repeat_actions_counter = repeat_actions_counter;
	}

	EnemyController& enemy_controller() {
		return world.enemy_controller();
	}

	TeamRelations& teams() {
		return world.teams();
	}

	CollisionAvoidance& collision_avoidance();
private:
	int handle_event(SDL_Event* event);

	/* Members */
	GameSettings settings;
	lua_State* L;
	int frame_n;
	GameStateInitData init_data;

	GameNetConnection connection;
	GameHud hud;
	GameView _view;
	GameWorld world;

	MTwist mtwist;
	font_data small_font, large_font;
	bool dragging_view;
	IOController iocontroller;

	//XXX: This repeats the last action N times, a more elegant solution is needed
	int repeat_actions_counter;
};

#endif /* GAMESTATE_H_ */
