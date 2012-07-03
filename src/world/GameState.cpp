/*
 * GameState.cpp:
 *  Handle to all the global game data.
 */

#include <SDL.h>
#include <cmath>
#include <SDL_opengl.h>
#include <cstring>
#include <ctime>
#include <vector>

#include "../data/game_data.h"
#include "../data/item_data.h"
#include "../data/tile_data.h"
#include "../data/dungeon_data.h"
#include "../data/class_data.h"

#include "../display/display.h"

#include "../util/math_util.h"

#include "../lua/lua_api.h"

#include "GameState.h"
#include "GameLevelState.h"

#include "net/GameNetConnection.h"

#include "objects/EnemyInst.h"
#include "objects/GameInst.h"
#include "objects/PlayerInst.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

GameState::GameState(const GameSettings& settings, lua_State* L, int vieww,
		int viewh, int hudw) :
		settings(settings), L(L), frame_n(0), chat(settings.username), hud(
				BBox(vieww, 0, vieww + hudw, viewh), BBox(0, 0, vieww, viewh)), view(
				0, 0, vieww, viewh), world(this) {
	memset(key_down_states, 0, sizeof(key_down_states));

	dragging_view = false;

	mouse_leftdown = false;
	mouse_rightdown = false;

	mouse_leftclick = false;
	mouse_rightclick = false;

	mouse_leftrelease = false;
	mouse_rightrelease = false;

	init_font(&small_font, settings.font.c_str(), 10);
	init_font(&large_font, settings.font.c_str(), 20);
}

GameState::~GameState() {
	release_font(&small_font);
	release_font(&large_font);

	lua_gc(L, LUA_GCCOLLECT, 0);
	lua_close(L);
}

void GameState::init_game() {
	time_t systime;
	time(&systime);
	int seed = systime;
	init_lua_data(this, L);

	if (settings.conntype == GameSettings::CLIENT) {
		char port_buffer[50];
		snprintf(port_buffer, 50, "%d", settings.port);
		connection.get_connection() = create_client_connection(
				settings.ip.c_str(), port_buffer);
	} else if (settings.conntype == GameSettings::HOST) {
		connection.get_connection() = create_server_connection(settings.port);
	}

	if (settings.conntype == GameSettings::CLIENT) {
		NetPacket packet;
		int tries = 0;
		while (true) {
			if (connection.get_connection()->get_next_packet(packet)) {
				seed = packet.get_int();
				break;
			} else if ((++tries) % 30000 == 0) {
				if (!update_iostate()) {
					exit(0);
				}
			}
		}
		printf("NETWORK: Recieving seed=0x%X\n", seed);
	} else if (settings.conntype == GameSettings::HOST) {
		NetPacket packet;
		packet.add_int(seed);
		packet.encode_header();
		connection.get_connection()->broadcast_packet(packet, true);
		printf("NETWORK: Broadcasting seed=0x%X\n", seed);
	}
	printf("Seed used for RNG = 0x%X\n", seed);
	mtwist.init_genrand(seed);
	set_level(world.get_level(0, true));

	GameInst* p = get_instance(get_level()->pc.local_playerid());
	window_view().sharp_center_on(p->x, p->y);

}

void GameState::set_level(GameLevelState* lvl) {
	world.set_current_level(lvl);
	if (lvl != NULL) {
		view.world_width = lvl->width;
		view.world_height = lvl->height;
	}
}

/*Handle new characters and exit signals*/
PlayerInst* GameState::local_player() {
	GameInst* player = get_instance(local_playerid());
	LANARTS_ASSERT(!player || dynamic_cast<PlayerInst*>(player));
	return (PlayerInst*) player;
}

int GameState::handle_event(SDL_Event *event) {
	int done = 0;

	if (get_level()) {
		if (dialogs.handle_event(this, event))
			return done;

		if (hud.handle_event(this, event))
			return done;

		if (chat.handle_event(this, event))
			return done;
	}

	switch (event->type) {
	case SDL_ACTIVEEVENT:
		break;

	case SDL_KEYDOWN: {
		if (event->key.keysym.sym == SDLK_ESCAPE) {
			done = 1;
		}
		key_down_states[event->key.keysym.sym] = 1;
		key_press_states[event->key.keysym.sym] = 1;
		break;
	}
	case SDL_KEYUP: {
		key_down_states[event->key.keysym.sym] = 0;
		break;
	}
	case SDL_MOUSEBUTTONDOWN: {
		if (event->button.button == SDL_BUTTON_LEFT) {
			mouse_leftdown = true;
			mouse_leftclick = true;
		} else if (event->button.button == SDL_BUTTON_RIGHT) {
			mouse_rightdown = true;
			mouse_rightclick = true;
		} else if (event->button.button == SDL_BUTTON_WHEELUP) {
			mouse_didupwheel = true;
		} else if (event->button.button == SDL_BUTTON_WHEELDOWN) {
			mouse_diddownwheel = true;
		}
		break;
	}
	case SDL_MOUSEBUTTONUP: {
		if (event->button.button == SDL_BUTTON_LEFT) {
			mouse_leftdown = false;
			mouse_leftrelease = true;
		} else if (event->button.button == SDL_BUTTON_RIGHT) {
			mouse_rightdown = false;
			mouse_rightrelease = true;
		}
		break;
	}
	case SDL_QUIT:
		done = 1;
		break;
	}
	return (done);
}
bool GameState::update_iostate(bool resetprev) {
	/*Reset everything to not-pressed and find out what is currently pressed*/
	SDL_Event event;
	if (resetprev) {
		memset(key_press_states, 0, sizeof(key_press_states));
		mouse_leftclick = false;
		mouse_rightclick = false;

		mouse_didupwheel = false;
		mouse_diddownwheel = false;

		mouse_leftrelease = false;
		mouse_rightrelease = false;
	}
	SDL_GetMouseState(&mousex, &mousey);
	while (SDL_PollEvent(&event)) {
		if (handle_event(&event))
			return false;
	}
	return true;
}
bool GameState::pre_step() {
	return world.pre_step();
}
void GameState::step() {
	chat.step(this);
	world.step(); //Has pointer to this (GameState) object
	lua_gc(L, LUA_GCSTEP, 0); // collect garbage incrementally
}

int GameState::key_down_state(int keyval) {
	return key_down_states[keyval];
}
int GameState::key_press_state(int keyval) {
	return key_press_states[keyval];
}

void GameState::adjust_view_to_dragging() {
	/*Adjust the view if the player is far from view center,
	 *if we are following the cursor, or if the minimap is clicked */
	bool is_dragged = false;
	if (key_down_state(SDLK_x)) {
		GameView& view = window_view();
		int nx = mouse_x() + view.x, ny = mouse_y() + view.y;
		for (int i = 0; i < 2; i++)
			view.center_on(nx, ny);
		is_dragged = true;
	} else {
		if (mouse_right_down()) {
			BBox minimap_bbox = hud.minimap_bbox(this);
			int mx = mouse_x() - minimap_bbox.x1, my = mouse_y()
					- minimap_bbox.y1;
			int mw = minimap_bbox.width(), mh = minimap_bbox.height();

			bool outofx = (mx < 0 || mx >= mw);
			bool outofy = (my < 0 || my >= mh);

			if (dragging_view || (!outofx && !outofy)) {
				view.sharp_center_on(mx * width() / mw, my * height() / mh);
				is_dragged = true;
			}
		}
	}

	/*If we were previously dragging, now snap back to the player position*/
	if (!is_dragged && dragging_view) {
		GameInst* p = get_instance(local_playerid());
		view.sharp_center_on(p->x, p->y);
	}
	dragging_view = is_dragged;
}
void GameState::draw(bool drawhud) {

	adjust_view_to_dragging();

	if (drawhud)
		gl_set_drawing_area(0, 0, view.width, view.height);
	else
		gl_set_drawing_area(0, 0, view.width + hud.width(), view.height);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	get_level()->tiles.pre_draw(this);

	get_level()->pc.update_fieldsofview(this);
	std::vector<GameInst*> safe_copy = get_level()->inst_set.to_vector();
	for (size_t i = 0; i < safe_copy.size(); i++) {
		safe_copy[i]->draw(this);
	}
	monster_controller().post_draw(this);
	get_level()->tiles.post_draw(this);
	if (drawhud) {
		hud.draw(this);
		chat.draw(this);
	}

	update_display();
	glFinish();
}

obj_id GameState::add_instance(GameInst *inst) {
	obj_id id = get_level()->inst_set.add_instance(inst);
	inst->init(this);
	return id;
}

void GameState::remove_instance(GameInst* inst) {
	get_level()->inst_set.remove_instance(inst);
	inst->deinit(this);
}

int GameState::width() {
	return get_level()->width;
}

int GameState::height() {
	return get_level()->height;
}

GameInst* GameState::get_instance(obj_id id) {
	return get_level()->inst_set.get_instance(id);
}

static bool circle_line_test(int px, int py, int qx, int qy, int cx, int cy,
		float radsqr) {
	int dx, dy, t, rt, ddist;
	dx = qx - px;
	dy = qy - py;
	ddist = dx * dx + dy * dy;
	t = -((px - cx) * dx + (py - cy) * dy);
	//;/ ddist;

	/* Restrict t to within the limits of the line segment */
	if (t < 0)
		t = 0;
	else if (t > ddist)
		t = ddist;

	dx = (px + t * (qx - px) / ddist) - cx;
	dy = (py + t * (qy - py) / ddist) - cy;
	rt = (dx * dx) + (dy * dy);
	return rt < (radsqr);
}

bool GameState::tile_radius_test(int x, int y, int rad, bool issolid, int ttype,
		Pos* hitloc) {
	int w = width() / TILE_SIZE, h = height() / TILE_SIZE;
	int distsqr = (TILE_SIZE / 2 + rad), radsqr = rad * rad;
	distsqr *= distsqr; //sqr it

	int mingrid_x = (x - rad) / TILE_SIZE, mingrid_y = (y - rad) / TILE_SIZE;
	int maxgrid_x = (x + rad) / TILE_SIZE, maxgrid_y = (y + rad) / TILE_SIZE;
	int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
	int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			Tile& tile = get_level()->tiles.get(xx, yy);
			bool istype = (tile.tile == ttype || ttype == -1);
			bool solidmatch = (get_level()->tiles.is_solid(xx, yy) == issolid);
			if (solidmatch && istype) {
				int offset = TILE_SIZE / 2; //To and from center
				int cx = int(xx * TILE_SIZE) + offset;
				int cy = int(yy * TILE_SIZE) + offset;
				int ydist = cy - y;
				int xdist = cx - x;
				double ddist = ydist * ydist + xdist * xdist;
				if (ddist < distsqr
						|| circle_line_test(cx - offset, cy - offset,
								cx + offset, cy - offset, x, y, radsqr)
						|| circle_line_test(cx - offset, cy - offset,
								cx - offset, cy + offset, x, y, radsqr)
						|| circle_line_test(cx - offset, cy + offset,
								cx + offset, cy + offset, x, y, radsqr)
						|| circle_line_test(cx + offset, cy - offset,
								cx + offset, cy + offset, x, y, radsqr)) {
					if (hitloc)
						*hitloc = Pos(xx, yy);
					return true;
				}
			}
		}
		//printf("\n");
	}
	return false;
}

int GameState::object_radius_test(GameInst* obj, GameInst** objs, int obj_cap,
		col_filterf f, int x, int y, int radius) {
	return get_level()->inst_set.object_radius_test(obj, objs, obj_cap, f, x, y,
			radius);
}
bool GameState::object_visible_test(GameInst* obj, PlayerInst* player,
		bool canreveal) {
	const int sub_sqrs = VISION_SUBSQRS;
	const int subsize = TILE_SIZE / sub_sqrs;

	int w = width() / subsize, h = height() / subsize;
	int x = obj->x, y = obj->y;
	int rad = obj->radius;
	int mingrid_x = (x - rad) / subsize, mingrid_y = (y - rad) / subsize;
	int maxgrid_x = (x + rad) / subsize, maxgrid_y = (y + rad) / subsize;
	int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
	int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);
	const std::vector<fov*>& fovs = player_controller().player_fovs();

	if ((canreveal && key_down_state(SDLK_BACKQUOTE)) || fovs.empty())
		return true;

	PlayerController& pc = player_controller();
	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			for (int i = 0; i < fovs.size(); i++) {
				bool isplayer = player == NULL
						|| (player->id == pc.player_ids()[i]);
				if (isplayer && fovs[i]->within_fov(xx, yy))
					return true;
			}
		}
	}
	return false;
}

void GameState::ensure_level_connectivity(int roomid1, int roomid2) {
	world.connect_entrance_to_exit(roomid1, roomid2);
}
void GameState::level_move(int id, int x, int y, int roomid1, int roomid2) {
	world.level_move(id, x, y, roomid1, roomid2);
}

bool GameState::mouse_left_click() {
	return mouse_leftclick;
}

/* Mouse click states */
bool GameState::mouse_right_click() {
	return mouse_rightclick;
}

bool GameState::mouse_left_down() {
	return mouse_leftdown;
}

bool GameState::mouse_right_down() {
	return mouse_rightdown;
}

bool GameState::mouse_left_release() {
	return mouse_leftrelease;
}

bool GameState::mouse_right_release() {
	return mouse_rightrelease;
}

bool GameState::mouse_upwheel() {
	return mouse_didupwheel;
}
bool GameState::mouse_downwheel() {
	return mouse_diddownwheel;
}

/* End mouse click states */

obj_id GameState::local_playerid() {
	return get_level()->pc.local_playerid();
}

GameTiles& GameState::tile_grid() {
	return get_level()->tiles;
}

MonsterController& GameState::monster_controller() {
	return get_level()->mc;
}

PlayerController& GameState::player_controller() {
	return get_level()->pc;
}
void GameState::skip_next_instance_id() {
	get_level()->inst_set.skip_next_id();
}
