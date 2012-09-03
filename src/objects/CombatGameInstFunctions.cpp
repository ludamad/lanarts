/*
 * CombatGameInstFunctions.cpp:
 *  Utility functions that operate on multiple CombatGameInst's
 */

#include <algorithm>
#include <cmath>

#include <rvo2/RVO.h>

#include "../gamestate/GameState.h"
#include "../gamestate/PlayerData.h"

#include "enemy/EnemyInst.h"
#include "player/PlayerInst.h"

#include "../util/math_util.h"
#include "collision_filters.h"

#include "CombatGameInstFunctions.h"

const int HUGE_DISTANCE = 1000000;
const int PATHING_RADIUS = 500;

/* Provides geometric distance between objects */
float inst_distance(CombatGameInst* inst1, CombatGameInst* inst2) {
	return distance_between(inst1->pos(), inst2->pos());
}

/* Provides the exact path cost to the player */
static float player_inst_path_distance(PlayerInst* player,
		CombatGameInst* inst) {
	PathInfo& p = player->path_to_player();
	PathingNode* pn = p.get(inst->x / TILE_SIZE, inst->y / TILE_SIZE);
	return pn->distance * TILE_SIZE;
}

/* Provides the best (lower bound) estimate possible cheaply */
float estimate_inst_path_distance(CombatGameInst* inst1,
		CombatGameInst* inst2) {
	PlayerInst* p;
	if ((p = dynamic_cast<PlayerInst*>(inst1))) {
		return player_inst_path_distance(p, inst2);
	} else if ((p = dynamic_cast<PlayerInst*>(inst2))) {
		return player_inst_path_distance(p, inst1);
	} else {
		return inst_distance(inst1, inst2);
	}
}

static bool player_inst_can_see(PlayerInst* player, CombatGameInst* inst2) {
	return player->within_field_of_view(inst2->pos());
}
bool inst_can_see(CombatGameInst *inst1, CombatGameInst *inst2) {
	PlayerInst* p;
	if ((p = dynamic_cast<PlayerInst*>(inst1))) {
		return player_inst_can_see(p, inst2);
	} else if ((p = dynamic_cast<PlayerInst*>(inst2))) {
		return player_inst_can_see(p, inst1);
	} else {
		return true;
	}
}

CombatGameInst* find_closest_hostile(GameState* gs, CombatGameInst* inst,
		const std::vector<CombatGameInst*>& candidates) {
	//Use a 'GameView' object to make use of its helper methods
	GameView view(0, 0, PATHING_RADIUS * 2, PATHING_RADIUS * 2, gs->width(),
			gs->height());

	TeamRelations& teams = gs->teams();
	float min_dist = HUGE_DISTANCE;
	CombatGameInst* closest_inst = NULL;

	for (int i = 0; i < candidates.size(); i++) {
		CombatGameInst* other = candidates[i];
		bool keep_chasing = false;
		if (other->id == inst->target()) {
			view.sharp_center_on(other->x, other->y);
			if (view.within_view(inst->bbox())) {
				keep_chasing = true;
			}

		}

		if (!keep_chasing && !inst_can_see(inst, candidates[i])) {
			continue;
		}
		if (!insts_are_hostile(teams, inst, candidates[i])) {
			continue;
		}
		float dist = inst_distance(inst, candidates[i]);
		if (dist < min_dist) {
			min_dist = dist;
			closest_inst = candidates[i];
		}
	}
	return closest_inst;
}
