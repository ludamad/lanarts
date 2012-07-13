/*
 * MonsterController.cpp:
 *  Centralized location of all pathing decisions of monsters, with collision avoidance
 */

#include <algorithm>
#include <cmath>

#include <rvo2/RVO.h>

#include "../GameState.h"
#include "MonsterController.h"
#include "PlayerController.h"

#include "../objects/EnemyInst.h"
#include "../objects/PlayerInst.h"

#include "../../combat_logic/attack_logic.h"

#include "../../util/colour_constants.h"
#include "../../util/math_util.h"
#include "../../util/world/collision_util.h"

#include "../../data/tile_data.h"
#include "../../data/weapon_data.h"

const int PATHING_RADIUS = 500;
const int HUGE_DISTANCE = 1000000;

MonsterController::MonsterController(bool wander) :
		monsters_wandering_flag(wander) {
	targetted = 0;
}

MonsterController::~MonsterController() {
	resize_paths(0);
}

void MonsterController::register_enemy(GameInst* enemy) {
	mids.push_back(enemy->id);
	RVO::Vector2 enemy_position(enemy->x, enemy->y);
	EnemyInst* e = (EnemyInst*)enemy;
	EffectiveStats& estats = e->effective_stats();
	EnemyBehaviour& eb = e->behaviour();

	eb.simulation_id = coll_avoid.add_object(e);
}

void towards_highest(PathInfo& path, Pos& p) {
	int highest;
	for (int y = -1; y <= +1; y++) {
		for (int x = -1; x <= +1; x++) {
			path.get(p.x + x, p.y + y);
		}
	}

}

void MonsterController::partial_copy_to(MonsterController & mc) const {
	mc.mids = this->mids;
	mc.resize_paths(0); //Automatically built in pre_step
	mc.player_simids.clear(); //Automatically built in pre_step
	mc.coll_avoid.clear();
	mc.targetted = this->targetted;
}

void MonsterController::finish_copy(GameLevelState* level) {
	for (int i = 0; i < mids.size(); i++) {
		EnemyInst* enemy = (EnemyInst*)level->inst_set.get_instance(mids[i]);
		EnemyBehaviour& eb = enemy->behaviour();
		if (!enemy)
			continue;
		int simid = coll_avoid.add_object(enemy);
		eb.simulation_id = simid;
	}
}

void MonsterController::deregister_enemy(EnemyInst* enemy) {
	coll_avoid.remove_object(enemy->behaviour().simulation_id);
}
void MonsterController::shift_target(GameState* gs) {
	if (!targetted)
		return; //Shouldn't auto-target if no targets are possible
	int i, j;
	for (i = 0; i < mids.size(); i++) {
		if (mids[i] == targetted)
			break;
	}
	PlayerInst* player = gs->local_player();

	for (j = i + 1; j % mids.size() != i; j++) {
		EnemyInst* e = (EnemyInst*)gs->get_instance(mids[j % mids.size()]);
		if (e == NULL)
			continue;

		bool isvisible = gs->object_visible_test(e, player, false);
		if (isvisible) {
			targetted = e->id;
			return;
		}
	}

}

int MonsterController::find_player_to_target(GameState* gs, EnemyInst* e) {
	//Use a 'GameView' object to make use of its helper methods
	GameView view(0, 0, PATHING_RADIUS * 2, PATHING_RADIUS * 2, gs->width(),
			gs->height());

	const std::vector<obj_id>& pids = gs->player_controller().player_ids();
	//Determine which players we are currently in view of
	BBox ebox = e->bbox();
	int mindistsqr = HUGE_DISTANCE;
	int closest_player_index = -1;
	for (int i = 0; i < pids.size(); i++) {
		PlayerInst* player = (PlayerInst*)gs->get_instance(pids[i]);
		bool isvisible = gs->object_visible_test(e, player, false);
		if (isvisible)
			((PlayerInst*)player)->rest_cooldown() = REST_COOLDOWN;
		view.sharp_center_on(player->x, player->y);
		bool chasing = e->behaviour().chase_timeout > 0
				&& pids[i] == e->behaviour().chasing_player;
		if (view.within_view(ebox) && (chasing || isvisible)) {
			e->behaviour().current_action = EnemyBehaviour::CHASING_PLAYER;

			int dx = e->x - player->x, dy = e->y - player->y;
			int distsqr = dx * dx + dy * dy;
			if (distsqr > 0 /*overflow check*/&& distsqr < mindistsqr) {
				mindistsqr = distsqr;
				closest_player_index = i;
			}
		}
	}
	return closest_player_index;
}
void MonsterController::process_players(GameState* gs) {
	const std::vector<obj_id>& pids = gs->player_controller().player_ids();

	if (player_simids.size() > pids.size()) {
		int diff = player_simids.size() - pids.size();
		for (int i = 0; i < diff; i++) {
			coll_avoid.remove_object(player_simids.back());
			player_simids.pop_back();
		}
	} else if (pids.size() > player_simids.size()) {
		int old_pids = player_simids.size();
		player_simids.resize(pids.size());
		for (int i = old_pids; i < gs->player_controller().player_ids().size();
				i++) {
			PlayerInst* p = (PlayerInst*)gs->get_instance(pids[i]);
			player_simids[i] = coll_avoid.add_player_object(p);
		}
	}

	//Create as many paths as there are players
	resize_paths(pids.size());
	for (int i = 0; i < pids.size(); i++) {
		GameInst* player = gs->get_instance(pids[i]);
		if (paths[i] == NULL)
			paths[i] = new PathInfo;
		paths[i]->calculate_path(gs, player->x, player->y, PATHING_RADIUS);
	}
}

void MonsterController::pre_step(GameState* gs) {

	PlayerInst* local_player = gs->local_player();
	std::vector<EnemyOfInterest> eois;

	process_players(gs);

	//Make sure targetted object is alive
	if (targetted && !gs->get_instance(targetted)) {
		targetted = 0;
	}

	//Update 'mids' to only hold live objects
	std::vector<obj_id> mids2;
	mids2.reserve(mids.size());
	mids.swap(mids2);
	for (int i = 0; i < mids2.size(); i++) {
		EnemyInst* e = (EnemyInst*)gs->get_instance(mids2[i]);
		if (e == NULL)
			continue;
		EnemyBehaviour& eb = e->behaviour();
		eb.step();

		bool isvisibleToLocal = gs->object_visible_test(e, local_player, false);
		if (isvisibleToLocal && !targetted)
			targetted = e->id;
		if (!isvisibleToLocal && targetted == e->id)
			targetted = 0;

		//Add live instances back to monster id list
		mids.push_back(mids2[i]);

		int closest_player_index = find_player_to_target(gs, e);

		if (eb.current_action == EnemyBehaviour::INACTIVE
				&& e->cooldowns().is_hurting()) {
			eb.current_action = EnemyBehaviour::CHASING_PLAYER;
		}
		if (closest_player_index == -1
				&& eb.current_action == EnemyBehaviour::CHASING_PLAYER) {
			eb.current_action = EnemyBehaviour::INACTIVE;
		}

		if (eb.current_action == EnemyBehaviour::CHASING_PLAYER)
			eois.push_back(EnemyOfInterest(e, closest_player_index));
		else if (eb.current_action == EnemyBehaviour::INACTIVE)
			monster_wandering(gs, e);
		else
			//if (eb.current_action == EnemyBehaviour::FOLLOWING_PATH)
			monster_follow_path(gs, e);
	}
	set_monster_headings(gs, eois);

	//Update player positions for collision avoidance simulator
	const std::vector<obj_id>& pids = gs->player_controller().player_ids();
	for (int i = 0; i < pids.size(); i++) {
		PlayerInst* p = (PlayerInst*)gs->get_instance(
				gs->player_controller().player_ids()[i]);
		coll_avoid.set_position(player_simids[i], p->x, p->y);
	}

	for (int i = 0; i < mids.size(); i++) {
		EnemyInst* e = (EnemyInst*)gs->get_instance(mids[i]);
		lua_gameinst_callback(gs->get_luastate(), e->etype().step_event, e);
		update_velocity(gs, e);
	}

	coll_avoid.step();

	for (int i = 0; i < mids.size(); i++) {
		EnemyInst* e = (EnemyInst*)gs->get_instance(mids[i]);
		update_position(gs, e);
	}
}

void MonsterController::update_velocity(GameState* gs, EnemyInst* e) {
	EnemyBehaviour& eb = e->behaviour();
	float movespeed = e->effective_stats().movespeed;

	if (e->cooldowns().is_hurting()) {
		e->vx /= 2, e->vy /= 2;
		movespeed /= 2;
	}

	coll_avoid.set_preferred_velocity(eb.simulation_id, e->vx, e->vy);
}
void MonsterController::update_position(GameState* gs, EnemyInst* e) {
	EnemyBehaviour& eb = e->behaviour();
	Posf pos = coll_avoid.get_position(eb.simulation_id);

	e->attempt_move_to_position(gs, pos.x, pos.y);
	coll_avoid.set_position(eb.simulation_id, pos.x, pos.y);
	coll_avoid.set_maxspeed(eb.simulation_id, e->effective_stats().movespeed);
}

void MonsterController::post_draw(GameState* gs) {
	EnemyInst* target = (EnemyInst*)gs->get_instance(targetted);
	if (!target)
		return;
	glLineWidth(2);
	gl_draw_circle(gs->window_view(), target->x, target->y,
			target->target_radius + 5, COL_GREEN.with_alpha(140), true);
	glLineWidth(1);
}

void MonsterController::clear() {
	resize_paths(0);
	mids.clear();
}
