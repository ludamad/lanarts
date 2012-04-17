/*
 * GameWorld.cpp
 *
 *  Created on: Apr 14, 2012
 *      Author: 100397561
 */

#include "GameWorld.h"
#include "GameState.h"
#include "GameLevelState.h"
#include "../data/dungeon_data.h"
#include "../data/tile_data.h"

GameWorld::GameWorld(int width, int height) :
		w(width), h(height), next_room_id(-1) {
}

GameWorld::~GameWorld() {
}

GameLevelState* GameWorld::get_level(int roomid) {
	if (roomid < level_states.size()) {
		level_states.resize(roomid + 1, NULL);
	}
	if (!level_states[roomid])
		level_states[roomid] = new GameLevelState(DNGN_MAIN_BRANCH, roomid, w,
				h);
	return level_states[roomid];
}

void GameWorld::step(GameState *gs) {
	if (next_room_id != -1){
		set_current_level(next_room_id);
		next_room_id = -1;
	}

	GameLevelState* current_level = lvl;
	current_level->simulate_count = 1000;
	for (int i = 0; i < level_states.size(); i++){
		if (level_states[i]->simulate_count > 0){
			//Set so that all the GameState getters are properly updated
			lvl = level_states[i];
			lvl->pc.pre_step(gs);
			lvl->mc.pre_step(gs);
			lvl->inst_set.step(gs);
			lvl->simulate_count--;
		}
	}
	lvl = current_level;
}

static int scan_entrance(const std::vector<GameLevelPortal>& portals,
		const Pos& tilepos) {
	for (int i = 0; i < portals.size(); i++) {
		if (portals[i].entrancesqr == tilepos) {
			return i;
		}
	}
	return -1;
}

void GameWorld::level_move(int id, int roomid1, int roomid2) {
	GameLevelState* state = get_level(roomid1);
	GameInst* inst = state->inst_set.get_by_id(id);
	Pos hitsqr;
	LANARTS_ASSERT(tile_radius_test(inst->x, inst->y, inst->radius, true, -1, &hitsqr, roomid1));

	int entr_n = scan_entrance(state->entrances, hitsqr);
	connect_entrance_to_exit(roomid1, roomid2);

	state->inst_set.remove(inst, false); // Remove but do not deallocate
	inst->id = 0; //Make ID null
	get_level(roomid2)->inst_set.add(inst);

}

void GameWorld::set_current_level(int roomid){
	lvl = get_level(roomid);
}
void GameWorld::set_current_level_lazy(int roomid){
	next_room_id = roomid;
}

void GameWorld::connect_entrance_to_exit(int roomid1, int roomid2) {
	GameLevelState* l1 = get_level(roomid1);
	GameLevelState* l2 = get_level(roomid2);
	LANARTS_ASSERT(l2->exits.size() == l1->entrances.size());
	for (int i = 0; i < l2->exits.size(); i++){
		l2->exits[i].exitsqr = l1->entrances[i].entrancesqr;
		l1->entrances[i].exitsqr = l2->exits[i].entrancesqr;
	}
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
static int squish(int a, int b, int c) {
	return std::min(std::max(a, b), c - 1);
}

bool GameWorld::tile_radius_test(int x, int y, int rad, bool issolid, int ttype, Pos *hitloc, int roomid){
	GameLevelState* lvl = get_level(roomid >= 0 ? roomid : current_room_id);
    int w = this->w / TILE_SIZE, h = this->h / TILE_SIZE;
    int distsqr = (TILE_SIZE / 2 + rad), radsqr = rad * rad;
    distsqr *= distsqr;
    int mingrid_x = (x - rad) / TILE_SIZE, mingrid_y = (y - rad) / TILE_SIZE;
    int maxgrid_x = (x + rad) / TILE_SIZE, maxgrid_y = (y + rad) / TILE_SIZE;
    int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
    int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);
    for(int yy = miny;yy <= maxy;yy++){
        for(int xx = minx;xx <= maxx;xx++){
            int tile = lvl->tiles.get(xx, yy);
            bool istype = (tile == ttype || ttype == -1);
            bool solidmatch = (game_tile_data[tile].solid == issolid);
            if(solidmatch && istype){
                int offset = TILE_SIZE / 2;
                int cx = int(xx * TILE_SIZE) + offset;
                int cy = int(yy * TILE_SIZE) + offset;
                int ydist = cy - y;
                int xdist = cx - x;
                double ddist = ydist * ydist + xdist * xdist;
                if(ddist < distsqr || circle_line_test(cx - offset, cy - offset, cx + offset, cy - offset, x, y, radsqr) || circle_line_test(cx - offset, cy - offset, cx - offset, cy + offset, x, y, radsqr) || circle_line_test(cx - offset, cy + offset, cx + offset, cy + offset, x, y, radsqr) || circle_line_test(cx + offset, cy - offset, cx + offset, cy + offset, x, y, radsqr)){
                    if(hitloc)
                        *hitloc = Pos(xx, yy);

                    return true;
                }
            }

        }

    }

    return false;
}


