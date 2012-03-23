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
#include "../data/tile_data.h"
#include "../data/dungeon_data.h"


GameState::GameState(int width, int height, int vieww, int viewh, int hudw) :
		world_width(width), world_height(height), level_number(1),  frame_n(0), hud(
				vieww, 0, hudw, viewh), view(50, 50, vieww, viewh, width,
				height) {
	memset(key_states, 0, sizeof(key_states));
	init_font(&pfont, "res/arial.ttf", 10);
	gennextstep = false;
	lvl = new GameLevelState(DNGN_MAIN_BRANCH, level_number, width, height);
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
		if (event->key.keysym.sym == SDLK_r) {

		}
		key_states[event->key.keysym.sym] = 1;
		break;
	}
	case SDL_KEYUP: {
		key_states[event->key.keysym.sym] = 0;
		break;
	}
	case SDL_MOUSEBUTTONDOWN: {
		if (event->button.button == SDL_BUTTON_LEFT)
			left_click = true;
		else if (event->button.button == SDL_BUTTON_RIGHT)
			right_click = true;
		break;
	}
	case SDL_MOUSEBUTTONUP: {
		if (event->button.button == SDL_BUTTON_LEFT)
			left_click = false;
		else if (event->button.button == SDL_BUTTON_RIGHT)
			right_click = false;
		break;
	}
	case SDL_QUIT:
		done = 1;
		break;
	}
	return (done);
}
bool GameState::step() {
	SDL_Event event;
	const int sub_sqrs = VISION_SUBSQRS;

	if (gennextstep)
		reset_level();

	//GameInst* player = this->player_obj();
	//if (player)
	//	pfov.calculate(this, player->last_x*sub_sqrs/TILE_SIZE, player->last_y*sub_sqrs/TILE_SIZE);

	//std::vector<GameInst*> safe_copy = inst_set.to_vector();
	//memset(key_states, 0, sizeof(key_states));
	SDL_GetMouseState(&mousex, &mousey);
	while (SDL_PollEvent(&event)) {
		if (handle_event(&event))
			return false;
	}
	frame_n++;
	lvl->pc.pre_step(this);
	lvl->mc.pre_step(this);
	lvl->inst_set.step(this);
	return true;
}

int GameState::key_press_state(int keyval) {
	return key_states[keyval];
}
void GameState::draw() {
	int vp[4];

	gl_set_drawing_area(0, 0, view.width, view.height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	lvl->tiles.pre_draw(this);
	std::vector<GameInst*> safe_copy = lvl->inst_set.to_vector();
	for (size_t i = 0; i < safe_copy.size(); i++) {
		safe_copy[i]->draw(this);
	}

	lvl->tiles.post_draw(this);
	hud.draw(this);
	update_display();
	glFinish();
}

obj_id GameState::add_instance(GameInst *inst) {
	obj_id id = lvl->inst_set.add(inst);
	inst->init(this);
	return id;
}

void GameState::remove_instance(GameInst* inst) {
	lvl->inst_set.remove(inst);
}

GameInst* GameState::get_instance(obj_id id) {
	return lvl->inst_set.get_by_id(id);
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
			int tile = lvl->tiles.get(xx, yy);
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
		col_filter f, int x, int y, int radius) {
	return lvl->inst_set.object_radius_test(obj, objs, obj_cap, f, x, y, radius);
}
bool GameState::object_visible_test(GameInst* obj) {
	const int sub_sqrs = VISION_SUBSQRS;
	const int subsize = TILE_SIZE / sub_sqrs;

	int w = width() / subsize, h = height()/ subsize;
	int x = obj->last_x, y = obj->last_y;
	int rad = obj->radius;
	int mingrid_x = (x - rad) / subsize, mingrid_y = (y - rad) / subsize;
	int maxgrid_x = (x + rad) / subsize, maxgrid_y = (y + rad) / subsize;
	int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
	int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);

//printf("minx=%d,miny=%d,maxx=%d,maxy=%d\n",minx,miny,maxx,maxy);

	for (int yy = miny; yy <= maxy; yy++) {
		for (int xx = minx; xx <= maxx; xx++) {
			for (int i = 0; i < lvl->pc.player_fovs().size(); i++) {
				if (lvl->pc.player_fovs()[i]->within_fov(xx, yy))
					return true;
			}
		}
	}
	return false;
}




void GameState::reset_level() {
	GeneratedLevel level;
	DungeonBranch& mainbranch = game_dungeon_data[DNGN_MAIN_BRANCH];
	int leveln = (level_number-1) % mainbranch.nlevels;

	std::vector<PlayerInst> playerinfo;
	std::vector<obj_id> pids =player_controller().player_ids();

	for (int i = 0; i < pids.size(); i++){
		PlayerInst* p = (PlayerInst*)get_instance(pids[i]);
		if (p->stats().hp > 0){
			playerinfo.push_back(*p);
			//TODO: Fix memory leak below
			/*for (int i = 0; i < level_states.size(); i++){
				delete level_states[i];
			}*/
		} else {
			level_number = 1;
			leveln = 0;
			level_states.clear();
		}
		remove_instance(p);
	}
//	lvl->inst_set.clear();
	player_controller().clear();
//	monster_controller().clear();

	Pos playersqr;
	GameLevelState* prevlvl = lvl;
	if (leveln >= level_states.size()){
		lvl = new GameLevelState(DNGN_MAIN_BRANCH, level_number, width(), height());
		level_states.push_back(lvl);

		generate_level(mainbranch.level_data[leveln], mtwist, level, this);
		if (playerinfo.size() == 0){
			//Generate player
			GameTiles& tiles = tile_grid();
			int start_x = (tiles.tile_width()-level.width())/2;
			int start_y = (tiles.tile_height()-level.height())/2;

			playersqr = generate_location(mtwist, level);
			playersqr.x += start_x;
			playersqr.y += start_y;
		} else {
			LANARTS_ASSERT(lvl->exits.size() == prevlvl->entrances.size());
			for (int i = 0; i < lvl->exits.size(); i++){
				lvl->exits[i].exitsqr = prevlvl->entrances[i].entrancesqr;
				prevlvl->entrances[i].exitsqr = lvl->exits[i].entrancesqr;
			}
		}
	} else {
		lvl = level_states[leveln];
	}
	if (playerinfo.size() == 0)
		playerinfo.push_back(PlayerInst(0,0));
	for (int i = 0; i < playerinfo.size(); i++){
		if (playerinfo[i].portal_used()){
			playersqr = playerinfo[i].portal_used()->exitsqr;
		}
		int px = (playersqr.x) * 32 + 16;
		int py = (playersqr.y) * 32 + 16;

		PlayerInst* p = new PlayerInst(playerinfo[i]);
		p->last_x = px, p->last_y = py;
		p->x = px, p->y = py;
		add_instance(p);
	}

	GameInst* p = get_instance(local_playerid());
	window_view().sharp_center_on(p->x, p->y);
	//Make sure we aren't going to regenerate the level next step
	gennextstep = false;
}


GameInst* GameState::nearest_object(GameInst* obj, int max_radius, col_filter f){
	return lvl->inst_set.object_nearest_test(obj, max_radius, f);
}




void GameState::set_level(int levelnum, bool reset) {
}
