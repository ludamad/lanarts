/*
 * MonsterControllerActions.cpp:
 *  Handles implementation of monster behaviours
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

const int HUGE_DISTANCE = 1000000;
const int OFFSCREEN_CHASE_TIME = 300;

static bool choose_random_direction(GameState* gs, EnemyInst* e, float& vx,
		float& vy) {
	const float deg2rad = 3.14159265f / 180.0f;
	int MAX_ATTEMPTS = 10;
	float movespeed = e->effective_stats().movespeed;
	MTwist& mt = gs->rng();

	for (int attempts = 0; attempts < MAX_ATTEMPTS; attempts++) {
		float direction = mt.rand(360) * deg2rad;
		vx = cos(direction), vy = sin(direction);
		int nx = round(e->rx + vx * TILE_SIZE), ny = round(
				e->ry + vy * TILE_SIZE);
		bool solid = gs->tile_radius_test(nx, ny, TILE_SIZE);
		if (!solid) {
			vx *= movespeed, vy *= movespeed;
			return true;
		}
	}
	return false;
}

bool has_ranged_attack(EnemyInst* e) {
	CombatStats& stats = e->stats();
	for (int i = 0; i < stats.attacks.size(); i++) {
		if (stats.attacks[i].is_ranged()) {
			return true;
		}
	}
	return false;
}
bool potentially_randomize_movement(GameState* gs, EnemyInst* e) {
	EnemyRandomization& er = e->behaviour().randomization;

	if (!has_ranged_attack(e)) {
		//Only enable this behaviour for ranged enemies for now
		return false;
	}

	bool randomized = false;
	if (er.should_randomize_movement()) {
		if (er.has_random_goal()) {
			randomized = true;
		}
		if (!randomized && gs->rng().rand(32) == 0
				&& choose_random_direction(gs, e, er.vx, er.vy)) {
			er.random_walk_timer = gs->rng().rand(TILE_SIZE, TILE_SIZE * 2);
			randomized = true;
		}
		if (randomized) {
			int nx = round(e->rx + er.vx), ny = round(e->ry + er.vy);
			bool solid = gs->tile_radius_test(nx, ny, TILE_SIZE);
			if (!solid) {
				e->vx = er.vx, e->vy = er.vy;
			} else {
				er.random_walk_timer = 0;
				er.successful_hit_timer = 0;
				er.damage_taken_timer = 0;
				randomized = false;
			}
		}
	}
	return randomized;
}

void MonsterController::set_monster_headings(GameState* gs,
		std::vector<EnemyOfInterest>& eois) {
	//Use a temporary 'GameView' object to make use of its helper methods
	PlayerController& pc = gs->player_controller();
	for (int i = 0; i < eois.size(); i++) {
		EnemyInst* e = eois[i].e;
		float movespeed = e->effective_stats().movespeed;
		int pind = eois[i].closest_player_index;
		obj_id pid = pc.player_ids()[pind];
		PlayerInst* p = (PlayerInst*)gs->get_instance(pid);
		EnemyBehaviour& eb = e->behaviour();

		eb.current_action = EnemyBehaviour::CHASING_PLAYER;
		eb.path.clear();
		if (gs->object_visible_test(e, p, false)) {
			eb.chase_timeout = OFFSCREEN_CHASE_TIME;
			eb.chasing_player = pid;
		}

		if (!potentially_randomize_movement(gs, e)) {
			paths[pind]->interpolated_direction(e->bbox(), movespeed, e->vx,
					e->vy);
		}

		//Compare position to player object
		float pdist = distance_between(Pos(e->x, e->y), Pos(p->x, p->y));

		AttackStats attack;
		bool viable_attack = attack_ai_choice(gs, e, p, attack);
		WeaponEntry& wentry = attack.weapon.weapon_entry();

		if (pdist < e->target_radius + p->target_radius) {
			e->vx = 0, e->vy = 0;
		}
		int mindist = wentry.range + e->target_radius - TILE_SIZE / 8;
		if (viable_attack) {
			int mindist = wentry.range + p->target_radius + e->target_radius
					- TILE_SIZE / 8;
			if (!attack.is_ranged()) {
				e->vx = 0, e->vy = 0;
			} else if (pdist < std::min(mindist, 40)) {
				e->vx = 0, e->vy = 0;
			}
			e->attack(gs, p, attack);

		}
	}
}

//returns true if will be exactly on target
static bool move_towards(EnemyInst* e, const Pos& p) {
	EnemyBehaviour& eb = e->behaviour();

	float speed = e->effective_stats().movespeed;
	float dx = p.x - e->x, dy = p.y - e->y;
	float mag = distance_between(p, Pos(e->x, e->y));

	if (mag <= speed / 2) {
		e->vx = dx;
		e->vy = dy;
		return true;
	}

	eb.path_steps++;
	e->vx = dx / mag * speed / 2;
	e->vy = dy / mag * speed / 2;

	return false;
}

void MonsterController::monster_follow_path(GameState* gs, EnemyInst* e) {
	MTwist& mt = gs->rng();
	float movespeed = e->effective_stats().movespeed;
	EnemyBehaviour& eb = e->behaviour();

	const int PATH_CHECK_INTERVAL = 600; //~10seconds
	float path_progress_threshold = movespeed / 50.0f;
	float progress = distance_between(eb.path_start, Pos(e->x, e->y));

	if (eb.path_steps > PATH_CHECK_INTERVAL
			&& progress / eb.path_steps < path_progress_threshold) {
		eb.path.clear();
		eb.current_action = EnemyBehaviour::INACTIVE;
		return;
	}

	if (eb.current_node < eb.path.size()) {
		if (move_towards(e, eb.path[eb.current_node]))
			eb.current_node++;

	} else {
		if (mt.rand(6) == 0) {
			std::reverse(eb.path.begin(), eb.path.end());
			eb.current_node = 0;
		} else
			eb.path.clear();
		eb.current_action = EnemyBehaviour::INACTIVE;
	}
}
void MonsterController::monster_wandering(GameState* gs, EnemyInst* e) {
	GameTiles& tile = gs->tile_grid();
	MTwist& mt = gs->rng();
	EnemyBehaviour& eb = e->behaviour();
	e->vx = 0, e->vy = 0;
	if (!monsters_wandering_flag)
		return;
	bool is_fullpath = true;
	if (eb.path_cooldown > 0) {
		eb.path_cooldown--;
		is_fullpath = false;
	}
	int ex = e->x / TILE_SIZE, ey = e->y / TILE_SIZE;

	do {
		int targx, targy;
		do {
			if (!is_fullpath) {
				targx = ex + mt.rand(-3, 4);
				targy = ey + mt.rand(-3, 4);
			} else {
				targx = mt.rand(tile.tile_width());
				targy = mt.rand(tile.tile_height());
			}
		} while (tile.is_solid(targx, targy));

		eb.current_node = 0;
		eb.path = astarcontext.calculate_AStar_path(gs, ex, ey, targx, targy);
		if (is_fullpath) {
			eb.path_cooldown = mt.rand(
					EnemyBehaviour::RANDOM_WALK_COOLDOWN * 2);
		}
		eb.current_action = EnemyBehaviour::FOLLOWING_PATH;
	} while (eb.path.size() <= 1);

	eb.path_steps = 0;
	eb.path_start = Pos(e->x, e->y);
}
