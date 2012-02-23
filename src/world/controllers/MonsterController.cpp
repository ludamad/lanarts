/*
 * MonsterController.cpp
 *
 *  Created on: Feb 20, 2012
 *      Author: 100397561
 */

#include <cmath>
#include "MonsterController.h"
#include "PlayerController.h"
#include "../objects/EnemyInst.h"
#include "../GameState.h"

const int PATHING_RADIUS = 500;

MonsterController::MonsterController() {
}

MonsterController::~MonsterController() {
}

void MonsterController::set_behaviours(GameState* gs) {
	//Use a temporary 'GameView' object to make use of its helper methods
	GameView view(0, 0, PATHING_RADIUS, PATHING_RADIUS, gs->width(),
			gs->height());
	PlayerController& pc = gs->player_controller();

	//Use the paths to determine enemy heading
	//At the same time, remove dead enemies
	std::vector<obj_id> mids2;
	mids2.reserve(mids.size());
	//Make 'mids' initially empty
	mids.swap(mids2);
	//iterate over old value of 'mids'
	for (int i = 0; i < mids2.size(); i++) {
		EnemyInst* e = (EnemyInst*) gs->get_instance(mids2[i]);
		//Skip destroyed instances
		if (e == NULL)
			continue;
		//Add live instances back to monster id list
		mids.push_back(mids2[i]);

		EnemyBehaviour& eb = e->behaviour();

		int xx = e->x - e->radius, yy = e->y - e->radius;
		int tx = e->x / TILE_SIZE, ty = e->y / TILE_SIZE;
		int w = e->radius * 2, h = e->radius * 2;
		int players = pc.player_ids().size();

		for (int i = 0; i < players; i++) {
			GameInst* player = gs->get_instance(pc.player_ids()[i]);
			view.sharp_center_on(player->x, player->y);

			if (view.within_view(xx, yy, w, h)) {
				eb.current_action = EnemyBehaviour::CHASING_PLAYER;
				paths[i].interpolated_direction(xx, yy, w, h, eb.speed, eb.vx,
						eb.vy);
				paths[i].stake_claim_from(tx, ty);
				//Compare position to player object
				int dx = player->last_x - e->last_x, dy = player->last_y
						- e->last_y;
				double abs = sqrt(dx * dx + dy * dy);
				if (abs < e->radius + player->radius)
					eb.vx = 0, eb.vy = 0;
			} else {
				eb.current_action = EnemyBehaviour::WANDERING;
				eb.vx = 0, eb.vy = 0;
			}
		}

	}
}

void MonsterController::pre_step(GameState* gs) {
	PlayerController& pc = gs->player_controller();
	const std::vector<obj_id> pids = pc.player_ids();

	//Create as many paths as there are players
	paths.resize(pids.size());
	for (int i = 0; i < pids.size(); i++) {
		GameInst* player = gs->get_instance(pids[i]);
		paths[i].calculate_path(gs, player->x, player->y, PATHING_RADIUS);
	}
	this->set_behaviours(gs);
}
