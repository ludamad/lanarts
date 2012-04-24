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
#include "../data/class_data.h"
#include "objects/PlayerInst.h"

GameWorld::GameWorld(GameState* gs, int width, int height) :
		game_state(gs), w(width), h(height), next_room_id(-1) {
	midstep = false;
	lvl = NULL;
}


GameWorld::~GameWorld() {
	reset();
}

void GameWorld::generate_room(GameLevelState* level){

}

int GameWorld::get_current_level_id()
{
    return game_state->level()->roomid;
}

void GameWorld::spawn_player(GeneratedLevel& genlevel, PlayerInst* inst){
	GameTiles& tiles = game_state->tile_grid();
	ClassType* c = &game_class_data[game_state->game_settings().classn];
	Pos epos;
	do {
		epos = generate_location(game_state->rng(), genlevel);
	} while (! genlevel.at(epos).passable || genlevel.at(epos).has_instance);

	int start_x = (tiles.tile_width() - genlevel.width()) / 2;
	int start_y = (tiles.tile_height() - genlevel.height()) / 2;

	genlevel.at(epos).has_instance = true;
	int px = (epos.x + start_x) * 32 + 16;
	int py = (epos.y + start_y) * 32 + 16;

	if (!inst){
		inst = new PlayerInst(c->starting_stats, px,py);
	} else {
		inst->last_x = px;
		inst->last_y = py;
		inst->x = px;
		inst->y = py;
	}

//	game_state->add_instance(inst);

	if (game_state->game_settings().conntype == GameSettings::CLIENT){
	game_state->add_instance(inst);
	game_state->add_instance(new PlayerInst(c->starting_stats, px+TILE_SIZE,py, false));
	} else {
		inst->last_x += TILE_SIZE;
		inst->x += TILE_SIZE;
		game_state->add_instance(new PlayerInst(c->starting_stats, px,py, false));
		game_state->add_instance(inst);

	}
}

GameLevelState* GameWorld::get_level(int roomid, bool spawnplayer, void** player_instances, size_t nplayers) {
	if (roomid >= level_states.size()) {
		level_states.resize(roomid + 1, NULL);
	}
	if (!level_states[roomid]){
		GameLevelState* currlvl = game_state->level();
		GameLevelState* newlvl = new GameLevelState(roomid, DNGN_MAIN_BRANCH, roomid, w,h);
		game_state->level() = newlvl;
		level_states[roomid] = game_state->level();

		DungeonBranch& mainbranch = game_dungeon_data[DNGN_MAIN_BRANCH];
		GeneratedLevel genlevel;
		generate_level(mainbranch.level_data[roomid], game_state->rng(), genlevel, game_state);
		game_state->level()->rooms = genlevel.rooms();
		if (spawnplayer){
			if (!player_instances)
				spawn_player(genlevel);
			else {
				for (int i =0; i < nplayers; i++){
					spawn_player(genlevel, (PlayerInst*)player_instances[i]);
				}
			}
		}
		game_state->level() = currlvl;//restore level context
	}
	return level_states[roomid];
}



void GameWorld::step() {
	redofirststep://I used a goto dont kill me

	
	const int STEPS_TO_SIMULATE = 1000;
	GameLevelState* current_level = game_state->level();
//	current_level->steps_left = STEPS_TO_SIMULATE;

    game_state->frame()++;
    
	midstep = true;
	for (int i = 0; i < level_states.size(); i++){
		if (level_states[i]->steps_left > 0){
			//Set so that all the GameState getters are properly updated
			game_state->level() = level_states[i];
			game_state->level()->pc.pre_step(game_state);
			game_state->level()->mc.pre_step(game_state);
			game_state->level()->inst_set.step(game_state);
			game_state->level()->steps_left--;
            game_state->level()->tiles.step(game_state);
		}
	}
	game_state->level() = current_level;

	midstep = false;
	if (next_room_id == -2){
		reset(0);
		next_room_id = 0;
	}
	if (next_room_id != -1){
		set_current_level(next_room_id);
		GameInst* g = game_state->get_instance(game_state->player_controller().local_playerid());
		game_state->window_view().sharp_center_on(g->x,g->y);
		next_room_id = -1;
		game_state->level()->steps_left = 1000;
// 		goto redofirststep;// goto top
	}
}

void GameWorld::regen_level(int roomid){
	GameLevelState* level = get_level(roomid);
	std::vector<PlayerInst*> player_cache;
	for (int i = 0; i < level->pc.player_ids().size(); i++){
		PlayerInst* p = (PlayerInst*)game_state->get_instance(level->pc.player_ids()[i]);
		game_state->remove_instance(p, false); // Remove but do not deallocate
		player_cache.push_back(p);
	}
	delete level;
	level_states[roomid] = NULL;
	GameLevelState* newlevel = get_level(roomid, true, (void**)&player_cache[0], player_cache.size());
	if (game_state->level() == level){
		game_state->level() = newlevel;
		game_state->level()->steps_left = 1000;
		GameInst* p = game_state->get_instance(game_state->local_playerid());
		game_state->window_view().sharp_center_on(p->x, p->y);
	}
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


void GameWorld::level_move(int id, int x, int y, int roomid1, int roomid2) {
	//save the level context
	GameLevelState* last = game_state->level();
	GameLevelState* state = get_level(roomid1);
	//set the level context
	game_state->level() = state;

	GameInst* inst = game_state->get_instance(id);

	Pos hitsqr;
	if (!inst) return;
	game_state->remove_instance(inst, false); // Remove but do not deallocate
	inst->last_x = x, inst->last_y = y;
	inst->x = x, inst->y = y;


	game_state->level() = get_level(roomid2);
	game_state->add_instance(inst);
	game_state->level()->steps_left = 1000;

	game_state->level() = last;
	PlayerInst* p;
	if ((p = dynamic_cast<PlayerInst*>(inst)) && p->is_local_focus()){
		set_current_level_lazy(roomid2);
	}

}

void GameWorld::set_current_level(int roomid){
	if (midstep) set_current_level_lazy(roomid);
	else
	game_state->level() = get_level(roomid);
}
void GameWorld::set_current_level_lazy(int roomid){
	next_room_id = roomid;
}

void GameWorld::reset(int keep){
	if (midstep) {
		next_room_id = -2;
	} else {
		for (int i = keep; i < level_states.size(); i++){
			delete level_states[i];
		}
		level_states.resize(keep);
		game_state->level() = get_level(keep, true /*spawn player*/);
	}
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
