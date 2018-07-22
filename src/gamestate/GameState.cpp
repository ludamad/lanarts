/*
 * GameState.cpp:
 *  Handle to all the global game data. Note this is somewhat of a 'god class'.
 *  This is tolerated mainly as it simply forwards a lot of calls to various components,
 *  without making the caller worry about which component does what.
 */

#include <lua.hpp>
#include <SDL.h>
#include <cmath>
#include <SDL_opengl.h>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <functional>

#include <lcommon/SerializeBuffer.h>
#include <lcommon/directory.h>
#include <lcommon/strformat.h>
#include <lcommon/sdl_headless_support.h>

#include <ldraw/display.h>
#include <luawrap/luawrap.h>

#include "draw/fonts.h"
#include "data/game_data.h"

#include "draw/draw_sprite.h"
#include "draw/TileEntry.h"

#include "lua_api/lua_api.h"
#include "lua_api/lua_api.h"

#include <luawrap/luawrap.h>
#include <luawrap/members.h>

#include <luawrap/macros.h>

#include "net/GameNetConnection.h"
#include "net-lib/lanarts_net.h"

#include "objects/EnemyInst.h"

#include "objects/PlayerInst.h"

#include "objects/GameInst.h"

#include "stats/ClassEntry.h"

#include "stats/items/ItemEntry.h"

#include "util/game_replays.h"
#include <lcommon/math_util.h>
#include <objects/InstTypeEnum.h>
#include <ldraw/draw.h>
#include <ldraw/colour_constants.h>

#include "GameMapState.h"
#include "GameState.h"
#include <iostream>
static int generate_seed() {
	//the most significant bits of systime are likely to be very similar, mix with clock()
	int clk = int(clock()) << 22;
	time_t systime;
	time(&systime);
	return clk ^ int(systime);
}

static const int GAME_SIDEBAR_WIDTH = 160;

GameState::GameState(const GameSettings& settings, lua_State* L) :
		settings(settings),
		L(L),
		connection(this),
		frame_n(0),
		world(this),
		repeat_actions_counter(0),
		config(L) {

	is_dragging_view = false;

	_game_timestamp = 0;

        const char* FIXED_SEED = getenv("LANARTS_SEED");
        if (FIXED_SEED == NULL) {
	    init_data.seed = generate_seed();
        } else {
            init_data.seed = atoi(FIXED_SEED);
        }
	rng_state_stack.push_back(&base_rng_state);
}

GameState::~GameState() {
}

void GameState::start_connection() {
    int count = 0;
    auto callback = [&]() {
        count++;
        if (count > 100) {
            count = 0;
            update_iostate(true, false);
            if(io_controller().user_has_requested_exit()){
                printf("User cancelled connection\n");
                return false;
            }
        }
        return true;
    };

	if (settings.conntype == GameSettings::SERVER) {
		printf("server connected\n");
		connection.initialize_as_server(callback, settings.port);
	} else if (settings.conntype == GameSettings::CLIENT) {
		connection.initialize_as_client(callback, settings.ip.c_str(), settings.port);
		printf("client connected\n");
		net_send_connection_affirm(connection, settings.username, settings.class_type);
	}
	if (settings.conntype == GameSettings::SERVER
			|| settings.conntype == GameSettings::NONE) {
	    player_data().register_player(settings.username, NULL, settings.class_type, LuaValue(), /* local player */ true);
            // Hardcoded for now:
            int n_extra_players = gamepad_states().size();
            if (getenv("LANARTS_CONTROLLER")) {
                n_extra_players -=1; // Remove one player if first player controller is desired
            }
            const char* classes[] = {
                getenv("L2") ? getenv("L2") : "Red Mage",
                getenv("L3") ? getenv("L3") : "Red Mage",
                getenv("L4") ? getenv("L4") : "Red Mage",
                getenv("L5") ? getenv("L5") : "Red Mage",
            };
            for (int i = 0 ; i < n_extra_players; i++) {
                std::string p = "Player ";
                p += char('0' + (i+2)); // Start at 'player 2'
                player_data().register_player(p, NULL, classes[i%4], LuaValue(), /* local player */ true);
            }
	}
}

static void _event_log_initialize(GameState* gs, GameSettings& settings) {

	std::string input_log_name_buffer;

	const char* input_log = NULL;
	const char* output_log = NULL;

	if (!settings.comparison_event_log.empty()) {
		input_log = settings.comparison_event_log.c_str();
	}

	if (settings.keep_event_log) {
		/* We will fail to write logs unless we have a log directory */
		ensure_directory("logs");
		format(input_log_name_buffer, "logs/game_event_log%d", gs->game_timestamp());
		output_log = input_log_name_buffer.c_str();
	}

	event_log_initialize(gs, input_log, output_log);
}

bool GameState::init_game() {
    if (settings.conntype == GameSettings::SERVER) {
        init_data.frame_action_repeat = settings.frame_action_repeat;
        init_data.network_debug_mode = settings.network_debug_mode;
        init_data.regen_on_death = settings.regen_on_death;
        init_data.time_per_step = settings.time_per_step;
        net_send_game_init_data(connection, player_data(), init_data);
        connection.set_accepting_connections(false);
    }
    if (!settings.loadreplay_file.empty()) {
        class_id class_type = -1;
        load_init(this, init_data.seed, class_type);
        settings.class_type = game_class_data.get(class_type).name;
    }
    if (!settings.savereplay_file.empty()) {
        class_id class_type = (int) game_class_data.get_id(settings.class_type);
        save_init(this, init_data.seed, class_type);
    }

    if (settings.conntype == GameSettings::CLIENT) {
        while (!init_data.received_init_data && !io_controller().user_has_requested_exit()) {
            connection.poll_messages(1 /* milliseconds */);
            if (!update_iostate(false)) {
                break;
            }
        }
        settings.frame_action_repeat = init_data.frame_action_repeat;
        settings.network_debug_mode = init_data.network_debug_mode;
        settings.regen_on_death = init_data.regen_on_death;
        settings.time_per_step = init_data.time_per_step;
    }

    initial_seed = init_data.seed;
    base_rng_state.init_genrand(init_data.seed);

    screens.clear(); // Clear previous screens
    return true;
}

void GameState::set_level(GameMapState* lvl) {
	world.set_current_level(lvl);
}

/*Handle new characters and exit signals*/
PlayerInst* GameState::local_player() {
	return screens.focus_object(this);
}

/*Handle new characters and exit signals*/
GameInst* GameState::focus_object() {
    GameInst* focus = local_player();
    if (focus == nullptr && get_level()) {
        for (auto id : monster_controller().monster_ids()) {
            focus = get_instance(id);
            if (focus != nullptr) {
                break;
            }

        }
    }
    return focus;
}

int GameState::object_radius_test(int x, int y, int radius, col_filterf f,
		GameInst** objs, int obj_cap) {
	return object_radius_test(NULL, objs, obj_cap, f, x, y, radius);
}

std::vector<PlayerInst*> GameState::players_in_level() {
	return player_data().players_in_level(world.get_current_level_id());
}

bool GameState::level_has_player() {
	return player_data().level_has_player(world.get_current_level_id());
}

void GameState::serialize(SerializeBuffer& serializer) {
	serializer.set_user_pointer(this);
	LuaSerializeConfig& conf = luaserialize_config();
    post_deserialize_data().clear();
	// Reset the serialization config:
	conf.reset();

    luawrap::globals(L)["Engine"]["pre_serialize"].push();
    luawrap::call<void>(L);

    LuaValue global_data = luawrap::globals(L)["package"]["loaded"]["core.GlobalData"];
//  Still true? //   Bug fix: If we just encoded normal global data, we would have problems with object caching.
    conf.encode(serializer, global_data);

	settings.serialize_gameplay_settings(serializer);

	serializer.write(base_rng_state); // Save RNG state
	serializer.write_int(_game_timestamp);
	serializer.write_int(initial_seed);

	serializer.write_int(this->frame_n);
	world.serialize(serializer);

	player_data().serialize(this, serializer);

    screens.serialize(this, serializer);
	luawrap::globals(L)["Engine"]["post_serialize"].push();
	luawrap::call<void>(L);

	serializer.flush();
}

void GameState::deserialize(SerializeBuffer& serializer) {
	serializer.set_user_pointer(this);
	LuaSerializeConfig& conf = luaserialize_config();
    post_deserialize_data().clear();
	// Reset the serialization config:
	conf.reset();

	luawrap::globals(L)["Engine"]["pre_deserialize"].push();
	luawrap::call<void>(L);

        LuaValue global_data;
        conf.decode(serializer, global_data);
        luawrap::globals(L)["package"]["loaded"]["core.GlobalData"] = global_data;

	settings.deserialize_gameplay_settings(serializer);
	serializer.read(base_rng_state); // Load RNG state
	serializer.read_int(_game_timestamp);
	serializer.read_int(initial_seed);


	serializer.read_int(this->frame_n);
	world.deserialize(serializer);
	player_data().deserialize(this, serializer);

	post_deserialize_data().process(this);
    screens.deserialize(this, serializer);

    bool first = true;
    screens.for_each_screen( [&]() {
        world.set_current_level(focus_object()->current_floor);
        view().sharp_center_on(focus_object()->ipos());
        if (first && local_player()) {
            settings.class_type = local_player()->class_stats().class_entry().name;
            first = false; // HACK to get first player's class
        }
    });
    luawrap::globals(L)["Engine"]["post_deserialize"].push();
    luawrap::call<void>(L);
    is_loading_save() = true;
}

obj_id GameState::add_instance(level_id level, GameInst* inst) {
	return game_world().get_level(level)->add_instance(this, inst);
}

GameInst* GameState::get_instance(level_id level, obj_id id) {
	return game_world().get_level(level)->game_inst_set().get_instance(id);
}

CollisionAvoidance & GameState::collision_avoidance() {
	return get_level()->collision_avoidance();
}

const ldraw::Font& GameState::font() {
	return res::font_primary();
}

const ldraw::Font& GameState::menu_font() {
	return res::font_menu();
}

int GameState::game_timestamp() {
	return _game_timestamp;
}

void GameState::renew_game_timestamp() {
	time_t systime;
	time(&systime);
	_game_timestamp = systime;
}

void GameState::start_game() {
        //// Restart RNG:
        //const char* FIXED_SEED = getenv("LANARTS_SEED");
        //if (FIXED_SEED == NULL) {
        //    init_data.seed += 1;
        //} else {
        //    init_data.seed = atoi(FIXED_SEED);
        //}
	//printf("Seed used for RNG = 0x%X\n", init_data.seed);
    int n_local_players = 0;
    for (PlayerDataEntry &player: player_data().all_players()) {
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

    // Reset screens:
    const int WIDTH = settings.view_width / n_x;
    const int HEIGHT = settings.view_height / n_y; // / N_PLAYERS;
    std::vector<BBox> bounding_boxes;
    for (PlayerDataEntry &player: player_data().all_players()) {
        const int x1 = (player.index % n_x) * WIDTH, y1 = (player.index / n_x) * HEIGHT;
        bounding_boxes.push_back(BBox {x1, y1, x1 + WIDTH, y1 + HEIGHT});
    }
    if (bounding_boxes.size() == 3) {
        bounding_boxes[1] = {WIDTH, 0, settings.view_width, settings.view_height};
    }

    screens.clear();
    for (PlayerDataEntry& player: player_data().all_players()) {
        if (!player.is_local_player) {
            continue;
        }
        BBox b = bounding_boxes.at(player.index);
        screens.add({-1, // index placeholder
                     GameHud {
                             BBox(b.x2 - GAME_SIDEBAR_WIDTH, b.y1, b.x2, b.y2),
                             BBox(b.x1, b.y1, b.x2 - GAME_SIDEBAR_WIDTH, b.y2)
                     }, // hud
                     GameView {0, 0, b.width() - GAME_SIDEBAR_WIDTH, b.height()}, // view
                     b, // window_region
                     player.index, // focus player id
                    });
    }

        // Reset game world state:
        loop("sound/overworld.ogg");
	if (game_world().number_of_levels() > 0) {
		game_world().reset();
	}
    // Reset player data state:
    player_data().reset();
    // Initialize event log for first_map_create:
	renew_game_timestamp();
	_event_log_initialize(this, settings);
	luawrap::globals(L)["Engine"]["first_map_create"].push();
	int levelid = luawrap::call<LuaValue>(L)["_id"].to_int();
    luawrap::globals(L)["table"]["copy"].push();
    lua_api::import(L, "InitialGlobalData").push();
    // Get the initial global data:
    lua_call(L, 0, 1);
    luawrap::globals(L)["package"]["loaded"]["core.GlobalData"].push();
    // Perform the copy:
    lua_call(L, 2, 0);
	set_level(game_world().get_level(levelid));
//		set_level(game_world().get_level(0, true));

    screens.for_each_screen( [&]() {
        GameInst *obj = focus_object();
        if (obj) {
            view().sharp_center_on(obj->x, obj->y);
        }
    });
}

int GameState::handle_event(SDL_Event* event, bool trigger_event_handling) {
	if (lua_api::luacall_handle_event(luastate(), event)) {
		return false;
	}

    bool should_exit = false;
    screens.for_each_screen( [&]() {
        GameMapState* level = get_level();
        if (trigger_event_handling && level && level->id() != -1) {
            if (game_hud().handle_event(this, event)) {
                should_exit = true;
            }
        }
    });

	return !should_exit && iocontroller.handle_event(event);
}

GameState* gs(SerializeBuffer& buff) {
	return (GameState*) buff.user_pointer();
}
bool GameState::update_iostate(bool resetprev, bool trigger_event_handling) {
	/* If 'resetprev', clear the io state for held keys
	 * and then poll what's currently pressed */
	if (repeat_actions_counter <= 0) {
		iocontroller.update_iostate(resetprev);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (handle_event(&event, trigger_event_handling)) {
				return false;
			}
		}
		/* Fire IOEvents for the current step*/
		LuaField io_callback = luawrap::globals(luastate())["Engine"]["io"];
		if (!io_callback.isnil()) {
			io_callback.push();
			luawrap::call<void>(luastate());
		}
	} else {
		iocontroller.update_iostate(false);
		repeat_actions_counter--;
	}

	return true;
}
bool GameState::pre_step(bool update_iostate) {
	return world.pre_step(update_iostate);
}

bool GameState::step() {
        //rng().init_genrand(initial_seed + frame_n);
	if (game_settings().network_debug_mode) {
		connection.check_integrity(this);
	}

        connection.poll_messages();

    screens.for_each_screen( [&](){
        game_hud().step(this);
    });
        // Handles frame_n incrementing and restarting:
	if (!world.step()) {
		return false;
	}
        // ROBUSTNESS:
        // Do not place logic here -- place in world.step()
        // before restart code.
        return true;
}

int GameState::key_down_state(int keyval) {
	return iocontroller.key_down_state(keyval);
}
int GameState::key_press_state(int keyval) {
	return iocontroller.key_press_state(keyval);
}

void GameState::adjust_view_to_dragging() {
	if (!is_dragging_view) {
		previous_view = view();
	}
	/*Adjust the view if the player is far from view center,
	 *if we are following the cursor, or if the minimap is clicked */
	bool is_dragged = false;

	if (mouse_right_down()) {
		BBox minimap_bbox = game_hud().minimap().minimap_bounds(this);
		if (is_dragging_view || minimap_bbox.contains(mouse_pos())) {
			Pos minimap_xy = mouse_pos() - minimap_bbox.left_top();
			Pos world_xy = game_hud().minimap().minimap_xy_to_world_xy(this, previous_view, minimap_xy);
            view().sharp_center_on(world_xy);
			is_dragged = true;
		}
	}

	/*If we were previously dragging, now snap back to the player position*/
	if (!is_dragged && is_dragging_view) {
		PlayerInst* p = local_player();
		if (p) {
            view().sharp_center_on(p->x, p->y);
		}
	}
	is_dragging_view = is_dragged;
}

static void lua_drawables_draw_below_depth(LuaDrawableQueue::Iterator iter, int depth) {
	for (; !iter.is_done() && iter.get_current_depth() < depth; iter.fetch_next()) {
		iter.draw_current();
	}
}

static void lua_drawables_draw_rest(LuaDrawableQueue::Iterator iter) {
	for (; !iter.is_done(); iter.fetch_next()) {
		iter.draw_current();
	}
}

void GameState::draw(bool drawhud) {
	perf_timer_begin(FUNCNAME);

    Timer t;
	ldraw::display_draw_start();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    screens.for_each_screen( [&]() {
        adjust_view_to_dragging();
        if (!get_level()){
            set_level(world.get_level(0));
        }

        //if (drawhud) {
        ldraw::display_set_window_region(screens.window_region());
        //} else {
        //    ldraw::display_set_window_region(
        //            BBoxF(0, 0, view().width + game_hud().width(), view().height));
        //}

        get_level()->tiles().pre_draw(this);

        std::vector<GameInst *> safe_copy = get_level()->game_inst_set().to_vector();
        LuaDrawableQueue::Iterator lua_drawables = get_level()->drawable_queue();

        for (size_t i = 0; i < safe_copy.size(); i++) {
            lua_drawables_draw_below_depth(lua_drawables, safe_copy[i]->depth);
            safe_copy[i]->draw(this);
        }
        lua_drawables_draw_rest(lua_drawables);

        lua_api::luacall_post_draw(L);

        monster_controller().post_draw(this);
        get_level()->tiles().post_draw(this);
        for (size_t i = 0; i < safe_copy.size(); i++) {
            safe_copy[i]->post_draw(this);
        }
        // Set drawing region to full screen:
        ldraw::display_set_window_region({0,0,game_settings().view_width, game_settings().view_height});
        if (drawhud) {
            game_hud().draw(this);
        }
        if (screens.amount() > 1) {
            ldraw::draw_rectangle_outline(COL_WHITE, screens.window_region());
        }
////        ldraw::display_set_window_region(screens.window_region());
    });
    lua_api::luacall_overlay_draw(L); // Used for debug purposes

	ldraw::display_draw_finish();

    // std::cout << "STEP MS " << float(t.get_microseconds()) / 1000.0f << std::endl;
//	glFinish(); // XXX: Apparently glFinish is not recommended
	perf_timer_end(FUNCNAME);
}

obj_id GameState::add_instance(GameInst* inst) {
	return get_level()->add_instance(this, inst);
}

void GameState::remove_instance(GameInst* inst, bool add_to_removed) {
	if (inst->destroyed) {
		return;
	}
	event_log(
			"Removing instance id: %d x: %f y: %f target_radius: %f depth %d\n",
			inst->id, inst->x, inst->y, inst->target_radius, inst->depth);
	GameMapState* level = world.get_level(inst->current_floor);
	level->game_inst_set().remove_instance(inst);
    if (add_to_removed) {
        game_world().register_removed_object(inst);
    }
	inst->deinit(this);
}

int GameState::width() {
	return get_level()->width();
}

int GameState::height() {
	return get_level()->height();
}

GameInst* GameState::get_instance(obj_id id) {
	return get_level()->game_inst_set().get_instance(id);
}

bool GameState::tile_radius_test(int x, int y, int rad, bool issolid, int ttype,
		Pos* hitloc) {
	return tiles().radius_test(Pos(x, y), rad, issolid, ttype, hitloc);
}

bool GameState::tile_line_test(const Pos& from_xy, const Pos& to_xy,
		bool issolid, int ttype, Pos* hitloc) {
	return tiles().line_test(from_xy, to_xy, issolid, ttype, hitloc);
}


//int GameState::object_square_test(GameInst** objs, int obj_cap,
//		col_filterf f, const Pos& pos) {
//	return get_level()->game_inst_set().object_square_test(objs, obj_cap, f, pos);
//}

int GameState::object_radius_test(GameInst* obj, GameInst** objs, int obj_cap,
		col_filterf f, int x, int y, int radius) {
	return get_level()->game_inst_set().object_radius_test(obj, objs, obj_cap,
			f, x, y, radius);
}

static bool player_radius_visible_test(PlayerInst* player, const BBox& bbox) {
	return player->field_of_view->within_fov(bbox);
}

bool GameState::radius_visible_test(int x, int y, int radius,
		PlayerInst* player, bool canreveal) {
	perf_timer_begin(FUNCNAME);

	int w = width() / TILE_SIZE, h = height() / TILE_SIZE;
	int mingrid_x = (x - radius) / TILE_SIZE, mingrid_y = (y - radius) / TILE_SIZE;
	int maxgrid_x = (x + radius) / TILE_SIZE, maxgrid_y = (y + radius) / TILE_SIZE;
	int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
	int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);

	if (player) {
		bool visible = false;
		if (player->current_floor == game_world().get_current_level_id()) {
			visible = player_radius_visible_test(player, BBox(minx, miny, maxx, maxy));
		}

		perf_timer_end(FUNCNAME);
		return visible;
	}

	bool has_player = false;
	PlayerData& pc = player_data();
	std::vector<PlayerDataEntry>& pdes = pc.all_players();
	for (int i = 0; i < pdes.size(); i++) {
		PlayerInst* p = (PlayerInst*)pdes[i].player_inst.get();
		if (p && p->current_floor == game_world().get_current_level_id()) {
			has_player = true;
			if (player_radius_visible_test(p, BBox(minx, miny, maxx, maxy))) {
				return true;
			}
		}
	}

	perf_timer_end(FUNCNAME);
	return (!has_player);
}
bool GameState::object_visible_test(GameInst* obj, PlayerInst* player,
		bool canreveal) {
	return radius_visible_test(obj->x, obj->y, obj->radius, player);
}

void GameState::ensure_level_connectivity(int roomid1, int roomid2) {
	world.connect_entrance_to_exit(roomid1, roomid2);
}
void GameState::level_move(int id, int x, int y, int roomid1, int roomid2) {
	world.level_move(id, x, y, roomid1, roomid2);
}

bool GameState::mouse_left_click() {
	return iocontroller.mouse_left_click();
}

/* Mouse click states */
bool GameState::mouse_right_click() {
	return iocontroller.mouse_right_click();
}

bool GameState::mouse_left_down() {
	return iocontroller.mouse_left_down();
}

bool GameState::mouse_right_down() {
	return iocontroller.mouse_right_down();
}

bool GameState::mouse_left_release() {
	return iocontroller.mouse_left_release();
}

bool GameState::mouse_right_release() {
	return iocontroller.mouse_right_release();
}

bool GameState::mouse_upwheel() {
	return iocontroller.mouse_upwheel();
}
bool GameState::mouse_downwheel() {
	return iocontroller.mouse_downwheel();
}

/* End mouse click states */

std::vector<IOGamepadState>& GameState::gamepad_states() {
    return iocontroller.gamepad_states();
}

GameTiles& GameState::tiles() {
	return get_level()->tiles();
}

MonsterController& GameState::monster_controller() {
	return get_level()->monster_controller();
}
void GameState::skip_next_instance_id() {
	get_level()->game_inst_set().skip_next_id();
}

level_id GameState::get_level_id() {
	if (get_level() == NULL) {
		return -1;
	}
	return get_level()->id();
}

PlayerDataEntry& GameState::local_player_data() {
    return screens.local_player_data(this);
}

GameScreen& GameState::get_screen(PlayerInst *player) {
    return screens.get_screen(this, player);
}

LuaValue& GameState::lua_input() {
	return local_player()->input_source().value;
}

static std::map<const char*, lsound::Sound> SOUND_MAP;

void play(lsound::Sound& sound, const char* path) {
        if (sound.empty()) {
            sound = lsound::load_sound(path);
        } 
        sound.play();
}

void play(const char* sound_path) {
    play(SOUND_MAP[sound_path], sound_path);
}

void loop(lsound::Sound& sound, const char* path) {
        if (sound.empty()) {
            sound = lsound::load_music(path);
        }
        lsound::stop_music();
        sound.loop();
}

void loop(const char* sound_path) {
    loop(SOUND_MAP[sound_path], sound_path);
}

void GameStatePostSerializeData::clear() {
    postponed_insts.clear();
    accepting_data = true;
}

void GameStatePostSerializeData::postpone_instance_deserialization(GameInst** holder, level_id current_floor, obj_id id) {
    LANARTS_ASSERT(accepting_data);
    postponed_insts.push_back({holder, current_floor, id});
}

void GameStatePostSerializeData::process(GameState* gs) {
    accepting_data = false;
    for (GameInstPostSerializeData& e : postponed_insts) {
        GameInst* inst = e.id > 0 ?
						 gs->get_level(e.current_floor)->game_inst_set().get_instance(e.id)
								  : gs->game_world().get_removed_object(-e.id).get();
        if (inst == NULL) {
            throw std::runtime_error(format("Attempt to load GameInst that cannot be found at level %d, id %d!", e.current_floor, e.id));
        } else {
            GameInst::retain_reference(inst);
            *e.holder = inst;
        }
        lua_State* L = gs->luastate();
        if (!inst->lua_variables.empty() && !inst->lua_variables.isnil()) {
            inst->lua_variables["__objectref"].push();
            GameInst** udata = (GameInst**) lua_touserdata(L, -1);
            LANARTS_ASSERT(inst == *udata);
            lua_pop(L, 1);
        }
    }
}
//
//void GameStatePostSerializeData::deserialize(SerializeBuffer &sb) {
//    accepting_data = false;
//    for (GameInst** ptr : inst_to_deserialize) {
//        InstType type;
//        int id;
//        sb.read_int(type);
//        sb.read_int(id);
//        GameInst*& inst = *ptr;
//        inst = from_inst_type(type);
//        inst->deserialize(gs(sb), sb);
//        inst->last_x = inst->x;
//        inst->last_y = inst->y;
//        inst->id = id;
//    }
//    for (GameInst** ptr : inst_to_deserialize) {
//        GameInst*& inst = *ptr;
//        inst->deserialize_lua(gs(sb), sb);
//    }
//}
//
//void GameStatePostSerializeData::serialize(SerializeBuffer &sb) {
//    accepting_data = false;
//    for (GameInst* inst : inst_to_serialize) {
//        sb.write_int(get_inst_type(inst));
//        sb.write_int(inst->id);
//        inst->serialize(gs(sb), sb);
//    }
//    for (GameInst* inst : inst_to_serialize) {
//        inst->serialize_lua(gs(sb), sb);
//    }
//}
//
//void GameStatePostSerializeData::postpone_destroyed_instance_deserialization(GameInst **holder) {
//    LANARTS_ASSERT(accepting_data);
//    inst_to_deserialize.push_back(holder);
//}
//
//void GameStatePostSerializeData::postpone_destroyed_instance_serialization(GameInst *inst) {
//    LANARTS_ASSERT(accepting_data);
//    inst_to_serialize.push_back(inst);
//}
