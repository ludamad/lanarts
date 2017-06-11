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
#include <functional>

#include <lcommon/smartptr.h>
#include <lcommon/mtwist.h>
#include <lcommon/luaserialize.h>

#include <ldraw/Font.h>

#include "fov/fov.h"

#include "net/GameNetConnection.h"

#include "GameLogger.h"
#include "GameSettings.h"
#include "GameView.h"

#include "GameWorld.h"
#include "IOController.h"
#include "PlayerData.h"
#include "Team.h"

#include <lsound/lsound.h>

#include "GameScreen.h"

struct lua_State;
class GameMapState;
class GameTiles;

class GameInst;
class PlayerInst;
class CollisionAvoidance;
class MonsterController;
class Serializer;

struct GameStateInitData {
	// Other than seed, other settings are not used in single-player.
	// They are used in multi-player to sync the server's settings.
	int seed, frame_action_repeat;
	bool regen_on_death, network_debug_mode, received_init_data;
	float time_per_step;
	GameStateInitData() :
					seed(0),
					frame_action_repeat(0),
					regen_on_death(false),
					network_debug_mode(false),
					received_init_data(false),
					time_per_step(0.0f) {
	}
};

class GameStatePostSerializeData {
public:
    void clear();
    void postpone_instance_deserialization(GameInst** holder, level_id current_floor, obj_id id);
    void process(GameState* gs);
private:
    struct GameInstPostSerializeData {
        GameInst** holder;
        level_id current_floor;
        obj_id id;
    };
    // Used to determine what to fix-up after serialization:
    std::vector<GameInstPostSerializeData> postponed_insts;
};

class GameState {
public:

	GameState(const GameSettings& settings, lua_State* L);
	~GameState();
	void start_connection();

	/* Call after construction, before game starts
	 * Returns false on failure */
	bool start_game();

	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);

	/* Primary events */
	void draw(bool drawhud = true);
	bool pre_step(bool update_iostate = true);

	/* Return false if game should exit*/
	bool step();
	bool update_iostate(bool resetprev = true, bool trigger_event_handling = true);

	void adjust_view_to_dragging();

	/* Instance retrieval and removal functions */
	GameInst* get_instance(obj_id id);
	template <typename T>
    T* get_instance(obj_id id) {
	    return dynamic_cast<T*>(get_instance(id));
	}
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
	bool tile_line_test(const Pos& from_xy, const Pos& to_xy,
			bool issolid = true, int ttype = -1, Pos* hitloc = NULL);
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
		return screens.view();
	}

	GameTiles& tiles();
	GameHud& game_hud() {
		return screens.hud();
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

	lua_State* luastate() {
		return L;
	}
	/* A single mersenne twister state is used for all random number generation */
	MTwist& rng() {
	    LANARTS_ASSERT(!rng_state_stack.empty());
		return *rng_state_stack.back();
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

	const ldraw::Font& font();

	const ldraw::Font& menu_font();

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

	/* Gamepad states */
    std::vector<IOGamepadState>& gamepad_states();

	/* Current frame number */
	int& frame() {
		return frame_n;
	}

	/* Keyboard press states */
	int key_down_state(int keyval);
	int key_press_state(int keyval);

	/* Level utility functions */
	GameMapState* get_level() {
		return world.get_current_level();
	}
	level_id get_level_id();
	GameMapState* get_level(level_id id) {
		return world.get_level(id);
	}

	void set_level(GameMapState* lvl);
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

	int game_timestamp();
	void renew_game_timestamp();
	void restart();

	CollisionAvoidance& collision_avoidance();

	LuaSerializeConfig& luaserialize_config() {
		return config;
	}
	TeamData& team_data() {
	    return game_world().team_data();
	}
	GameStatePostSerializeData& post_deserialize_data() {
	    return _post_deserialize_data;
	}
	template <typename Func>
	void for_screens(Func&& f) {
	    screens.for_each_screen(f);
	}
private:
	int handle_event(SDL_Event* event, bool trigger_event_handling = true);

	int _game_timestamp;

	/* Members */
	GameSettings settings;
	lua_State* L;
	int frame_n;
        // Used for restarting purposes, the initial value held by core.GlobalData, decided by runtime/InitialGlobalData.lua:
        LuaValue initial_global_data;
	GameStateInitData init_data;

	GameNetConnection connection;
	GameWorld world;

	// Maintain a LIFO stack of RNG states so that portions of game-play can isolate from each other.
	std::vector<MTwist*> rng_state_stack;
    MTwist base_rng_state;

    // Game screens to draw:
    GameScreenSet screens;
	// For dragging purposes:
	GameView previous_view;
	bool is_dragging_view;
	IOController iocontroller;

	//XXX: This repeats the last action N times, a more elegant solution is needed
	int repeat_actions_counter;

	LuaSerializeConfig config;
	GameStatePostSerializeData _post_deserialize_data;
        int initial_seed = 0;
    friend class GameWorld;
};

void play(const char* sound_path);
void loop(const char* sound_path);

#endif /* GAMESTATE_H_ */
