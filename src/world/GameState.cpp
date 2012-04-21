/*
 * GameState.cpp
 *
 *  Created on: 2011-09-29
 *      Author: 100397561
 */

#include "GameState.h"
#include "../display/display.h"
#include <SDL.h>
#include <cmath>
#include <SDL_opengl.h>
#include <cstring>
#include "objects/EnemyInst.h"
#include "objects/PlayerInst.h"
#include <ctime>
#include <vector>

#include "../procedural/levelgen.h"
#include "../data/item_data.h"
#include "../data/tile_data.h"
#include "../data/dungeon_data.h"
#include "../data/class_data.h"
#include <ctime>

#include "net/GameNetConnection.h"

GameState::GameState(const GameSettings& settings, int width, int height, int vieww, int viewh, int hudw) :
		settings(settings), world_width(width), world_height(height),  frame_n(0),
				hud(vieww, 0, hudw, viewh),
				view(50, 50, vieww, viewh, width, height),
				world(this, width, height),
				mouse_leftdown(0), mouse_rightdown(0),
				mouse_leftclick(0), mouse_rightclick(0) {
	memset(key_down_states, 0, sizeof(key_down_states));
	init_font(&pfont, "res/arial.ttf", 10);
	time_t t;
	time(&t);
	mtwist.init_genrand(t);
	gennextstep = false;

	if (settings.conntype == GameSettings::CLIENT){
		char port_buffer[50];
		snprintf(port_buffer, 50, "%d", settings.port);
		connection.get_connection() = create_client_connection(settings.ip.c_str(), port_buffer);
	} else if (settings.conntype == GameSettings::HOST){
		connection.get_connection() = create_server_connection(settings.port);
	}

	level() = world.get_level(0, true);
	GameInst* p = get_instance(level()->pc.local_playerid());
	window_view().sharp_center_on(p->x, p->y);
}

GameState::~GameState() {
	release_font(&pfont);
}

/*Handle new characters and exit signals*/
int GameState::handle_event(SDL_Event *event) {
	int done;

	done = 0;

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
		if (event->button.button == SDL_BUTTON_LEFT){
			mouse_leftdown = true;
			mouse_leftclick = true;
		}else if (event->button.button == SDL_BUTTON_RIGHT){
			mouse_rightdown = true;
			mouse_rightclick = true;
		}else if (event->button.button == SDL_BUTTON_WHEELUP){
			mouse_didupwheel = true;
		}else if (event->button.button == SDL_BUTTON_WHEELDOWN){
			mouse_diddownwheel = true;
		}
		break;
	}
	case SDL_MOUSEBUTTONUP: {
		if (event->button.button == SDL_BUTTON_LEFT)
			mouse_leftdown = false;
		else if (event->button.button == SDL_BUTTON_RIGHT)
			mouse_rightdown = false;
		break;
	}
	case SDL_QUIT:
		done = 1;
		break;
	}
	return (done);
}
bool GameState::update_iostate(){
	SDL_Event event;
	memset(key_press_states, 0, sizeof(key_press_states));
	SDL_GetMouseState(&mousex, &mousey);
	mouse_leftclick = false;
	mouse_rightclick = false;
	mouse_didupwheel = false;
	mouse_diddownwheel = false;
	while (SDL_PollEvent(&event)) {
		if (handle_event(&event))
			return false;
	}
	return true;
}
bool GameState::step() {
	const int sub_sqrs = VISION_SUBSQRS;

	if (!update_iostate())
		return false;

	frame_n++;
	world.step();
	return true;
}

int GameState::key_down_state(int keyval) {
	return key_down_states[keyval];
}
int GameState::key_press_state(int keyval) {
	return key_press_states[keyval];
}
void GameState::draw() {
	int vp[4];

	gl_set_drawing_area(0, 0, view.width, view.height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	level()->tiles.pre_draw(this);
	std::vector<GameInst*> safe_copy = level()->inst_set.to_vector();
	for (size_t i = 0; i < safe_copy.size(); i++) {
		safe_copy[i]->draw(this);
	}
	monster_controller().post_draw(this);
	level()->tiles.post_draw(this);
	hud.draw(this);
	update_display();
	glFinish();
}

obj_id GameState::add_instance(GameInst *inst) {
	obj_id id = level()->inst_set.add(inst);
	inst->init(this);
	return id;
}

void GameState::remove_instance(GameInst* inst, bool deallocate) {
	level()->inst_set.remove(inst, deallocate);
	inst->deinit(this);
}

GameInst* GameState::get_instance(obj_id id) {
	return level()->inst_set.get_by_id(id);
}

static int squish(int a, int b, int c) {
	return std::min(std::max(a, b), c - 1);
}

static bool sqr_line_test(int x, int y, int w, int h, int sx, int sy,
		int size) {

}
bool GameState::tile_line_test(int x, int y, int w, int h) {
	int sx = x / TILE_SIZE, sy = y / TILE_SIZE;
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
//game_id GameState::collides_with(){
//}

bool GameState::tile_radius_test(int x, int y, int rad, bool issolid,
		int ttype, Pos* hitloc) {
	int w = width()/ TILE_SIZE, h = height() / TILE_SIZE;
	//(rad*2) **2 area
	//should test x, y positions filling in circle
	int distsqr = (TILE_SIZE / 2 + rad), radsqr = rad * rad;
	distsqr *= distsqr; //sqr it

	int mingrid_x = (x - rad) / TILE_SIZE, mingrid_y = (y - rad) / TILE_SIZE;
	int maxgrid_x = (x + rad) / TILE_SIZE, maxgrid_y = (y + rad) / TILE_SIZE;
	int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
	int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);

	//printf("minx=%d,miny=%d,maxx=%d,maxy=%d\n",minx,miny,maxx,maxy);

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			int tile = level()->tiles.get(xx, yy);
			bool istype = (tile == ttype || ttype == -1);
			bool solidmatch = (game_tile_data[tile].solid == issolid);
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
					if (hitloc) *hitloc = Pos(xx,yy);
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
	return level()->inst_set.object_radius_test(obj, objs, obj_cap, f, x, y, radius);
}
bool GameState::object_visible_test(GameInst* obj) {
	const int sub_sqrs = VISION_SUBSQRS;
	const int subsize = TILE_SIZE / sub_sqrs;

	int w = width() / subsize, h = height()/ subsize;
	int x = obj->x, y = obj->y;
	int rad = obj->radius;
	int mingrid_x = (x - rad) / subsize, mingrid_y = (y - rad) / subsize;
	int maxgrid_x = (x + rad) / subsize, maxgrid_y = (y + rad) / subsize;
	int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
	int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);
	const std::vector<fov*>& fovs = level()->pc.player_fovs();

	if (fovs.empty()) return true;

//printf("minx=%d,miny=%d,maxx=%d,maxy=%d\n",minx,miny,maxx,maxy);

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			for (int i = 0; i < fovs.size(); i++) {
				if (fovs[i]->within_fov(xx, yy))
					return true;
			}
		}
	}
	return false;
}

void GameState::ensure_connectivity(int roomid1, int roomid2){
	world.connect_entrance_to_exit(roomid1, roomid2);
}
void GameState::level_move(int id, int x, int y, int roomid1, int roomid2){
	world.level_move(id, x, y, roomid1, roomid2);
}

GameInst* GameState::nearest_object(GameInst* obj, int max_radius, col_filterf f){
	return level()->inst_set.object_nearest_test(obj, max_radius, f);
}

