/*
 * MonsterControllerActions.cpp:
 *  Handles implementation of monster behaviours
 */

#include <algorithm>
#include <cmath>

#include <rvo2/RVO.h>

#include "draw/colour_constants.h"
#include "draw/TileEntry.h"
#include "gamestate/GameState.h"

#include "gamestate/PlayerData.h"

#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"
#include "stats/effect_data.h"

#include <lcommon/math_util.h>

#include "objects/PlayerInst.h"

#include "objects/collision_filters.h"
#include "EnemyInst.h"
#include "MonsterController.h"

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

        bool should_randomize_movement = er.should_randomize_movement();
        if (e->etype().name == "Adder" || e->etype().name == "Ogre Mage" || e->etype().name == "Black Mamba") {
                should_randomize_movement = true; // Always random
        } else if (!has_ranged_attack(e) && gs->get_level()->label() != "Plain Valley") {
		//Only enable this behaviour for ranged enemies for now, unless in overworld
                should_randomize_movement = false;
	} else if (!has_ranged_attack(e) && e->effective_stats().movespeed <= 3) {
                // Want it on fast melee units only
                should_randomize_movement = false;
        } else if (e->effective_stats().movespeed <= 1.5) {
                // Dont want it on really slow units
                should_randomize_movement = false;
        }

	bool randomized = false;
	if (should_randomize_movement) {
		if (er.has_random_goal()) {
			randomized = true;
		}
		if (!randomized && gs->rng().rand(32) == 0
				&& choose_random_direction(gs, e, er.vx, er.vy)) {
			er.random_walk_timer = gs->rng().rand(TILE_SIZE, TILE_SIZE * 4) / e->effective_stats().movespeed;
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

static bool same_target_and_moved_colfilter(GameInst* self, GameInst* other) {
	EnemyInst* e1 = (EnemyInst*)self;
	EnemyInst* e2 = dynamic_cast<EnemyInst*>(other);
	if (!e2) {
		return false;
	}
	if (e1->behaviour().chasing_actor != e2->behaviour().chasing_actor) {
		return false;
	}
	return (e2->behaviour().movement_decided);
}

const int TOO_LARGE_RANGE = 99999;
static bool attack_ai_choice(GameState* gs, CombatGameInst* inst,
		CombatGameInst* target, AttackStats& attack) {
	CombatStats& stats = inst->stats();
	std::vector<AttackStats>& attacks = stats.attacks;

	int attack_id = -1;
	int smallest_range = TOO_LARGE_RANGE;
	float dist = distance_between(Pos(inst->x, inst->y),
			Pos(target->x, target->y));
	int radii = inst->target_radius + target->target_radius;

	for (int i = 0; i < attacks.size(); i++) {
		WeaponEntry& wentry = attacks[i].weapon.weapon_entry();
		int range = wentry.range();
		if (!attacks[i].projectile.empty()) {
			ProjectileEntry& pentry = attacks[i].projectile_entry();
			range = std::max(range, pentry.range());
		}
		if (radii + range >= dist && range < smallest_range) {
			attack_id = i;
			smallest_range = range;
		}
	}

	if (attack_id > -1) {
		attack = attacks[attack_id];
		return true;
	}
	return false;
}
static bool go_towards_if_free_in_direction(GameState* gs, CombatGameInst* inst,
		float vx, float vy) {
	float tx = vx, ty = vy;
	normalize(tx, ty, TILE_SIZE);
	normalize(vx, vy, inst->effective_stats().movespeed);
	float nx = inst->rx + tx, ny = inst->ry + ty;
	if (!gs->solid_test(inst, round(nx), round(ny), inst->radius)) {
		inst->vx = vx, inst->vy = vy;
		return true;
	}
	return false;
}

void MonsterController::set_monster_headings(GameState* gs,
		std::vector<EnemyOfInterest>& eois) {
	perf_timer_begin(FUNCNAME);

	std::sort(eois.begin(), eois.end());
	for (int i = 0; i < eois.size(); i++) {
		eois[i].e->behaviour().movement_decided = false;
	}
	for (int i = 0; i < eois.size(); i++) {
		EnemyInst* e = eois[i].e;
		float movespeed = e->effective_stats().movespeed;
		int actor_id = eois[i].actor_id;
		CombatGameInst* p = gs->get_instance<CombatGameInst>(actor_id);
                if (p == NULL) {
                    continue;
                }
		EnemyBehaviour& eb = e->behaviour();

		eb.current_action = EnemyBehaviour::CHASING_PLAYER;
		eb.path.clear();
		if (is_visible(gs, e, p)) {
			eb.chase_timeout = OFFSCREEN_CHASE_TIME;
			eb.chasing_actor = p->id;
		}

		if (!potentially_randomize_movement(gs, e)) {
                    if (e->has_paths_data()) {
                        // TODO break out allied monster code into its own thing:
                        e->vx = 0, e->vy = 0;
                        if (e->has_paths_data()) {
                            Pos p = e->direction_towards_enemy(gs);
                            e->vx = p.x, e->vy = p.y;
                            float speed = e->effective_stats().movespeed;
                            normalize(e->vx, e->vy, speed);
                            if (e->vx == 0 && e->vy == 0) {
                                GameInst* inst = get_nearest_ally(gs, e);
                                if (inst) {
                                    if (distance_between(e->ipos(), inst->ipos()) > TILE_SIZE) {
                                        Pos p = e->direction_towards_ally_player(gs);
                                        e->vx = p.x, e->vy = p.y;
                                        float speed = e->effective_stats().movespeed;
                                        normalize(e->vx, e->vy, speed);
                                    }
                                }
                            }
                        }
                    } else {
                        PosF heading = get_direction_towards(gs, e, p, movespeed);
                        e->vx = heading.x;
                        e->vy = heading.y;
                    }
		}

		//Compare position to player object
		float pdist = distance_between(Pos(e->x, e->y), Pos(p->x, p->y));

		AttackStats attack;
		bool viable_attack = attack_ai_choice(gs, e, p, attack);
		WeaponEntry& wentry = attack.weapon_entry();
		bool hasproj = attack.projectile.id != NO_ITEM;

		// Part of: Implement a bunch of new status effects.
                bool has_fear = (e->effects().get(get_effect_by_name("Fear")));
		if (pdist < e->target_radius + p->target_radius && !has_fear) {
			e->vx = 0, e->vy = 0;
		}

		if (viable_attack) {
			int mindist = wentry.range() + p->target_radius + e->target_radius
					- TILE_SIZE / 8;
			if (hasproj) {
				mindist = attack.projectile_entry().range();
			}
			if (has_fear) {
			    // Don't stop moving near if have fear.
			} else if (!attack.is_ranged()) {
				e->vx = 0, e->vy = 0;
			} else {
				int close = 40;
                                bool stop_once_in_range = (e->effects().get(get_effect_by_name("StopOnceInRange")));
				if (stop_once_in_range) {
					close = 150;
				}
				if (pdist < std::min(mindist, close)) {
					e->vx = 0, e->vy = 0;
				}
			}
			e->attack(gs, p, attack);

		}
		if (gs->tile_radius_test(e->x, e->y, TILE_SIZE / 2 + 4)) {
			if (gs->object_radius_test(e, NULL, 0,
					same_target_and_moved_colfilter, e->x, e->y,
					e->target_radius - e->effective_stats().movespeed - 2)) {
				float dx = p->rx - e->rx, dy = p->ry - e->ry;
				if (!go_towards_if_free_in_direction(gs, e, -dy, dx)
						&& !go_towards_if_free_in_direction(gs, e, dy, -dx)) {
					e->vx = 0, e->vy = 0;
				}
			}
		}

		eb.movement_decided = true;
		e->vx = round(e->vx * 4096.0f) / 4096.0f;
		e->vy = round(e->vy * 4096.0f) / 4096.0f;
	}

	perf_timer_end(FUNCNAME);
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
	// Ensure floating point differences do not occur
	e->vx = round(e->vx * 4096.0f) / 4096.0f;
	e->vy = round(e->vy * 4096.0f) / 4096.0f;

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
	GameTiles& tile = gs->tiles();
	MTwist& mt = gs->rng();
	EnemyBehaviour& eb = e->behaviour();
	e->vx = 0, e->vy = 0;

        // Part of: Implement status effects.
    bool forced_wander = (e->effects().get(get_effect_by_name("Dazed")));

	if (!forced_wander && !monsters_wandering_flag) {
		return;
	}
	perf_timer_begin(FUNCNAME);
	bool is_fullpath = false;
	if (eb.path_cooldown > 0) {
		eb.path_cooldown--;
		is_fullpath = false;
	}
	int ex = e->x / TILE_SIZE, ey = e->y / TILE_SIZE;

	Pos target;
	int tries_left = 35; // arbitrary number
	do {
		if (!is_fullpath) {
			target.x = squish(ex + mt.rand(-3, 4), 0, tile.tile_width() - 1);
			target.y = squish(ey + mt.rand(-3, 4), 0, tile.tile_height() - 1);
		} else {
			target.x = mt.rand(tile.tile_width());
			target.y = mt.rand(tile.tile_height());
		}
		tries_left--;
	} while (tries_left > 0 && tile.is_solid(target));

	if (tries_left == 0) {
		return; // Avoid pathological cases
	}

	Pos exy = Pos(ex, ey);
	eb.path = astarcontext.calculate_AStar_path(gs, exy, target, AStarPath::surrounding_region(exy, target, Size(6, 6)));
	if (eb.path.size() <= 1) {
		return;
	}
	eb.current_node = 0;
	if (is_fullpath) {
		eb.path_cooldown = EnemyBehaviour::RANDOM_WALK_COOLDOWN/2 + mt.rand(EnemyBehaviour::RANDOM_WALK_COOLDOWN);
	}
	eb.current_action = EnemyBehaviour::FOLLOWING_PATH;

	eb.path_steps = 0;
	eb.path_start = Pos(e->x, e->y);

	event_log(
			"Path for instance id: %d, (%d path steps), x: %d y: %d target_radius: %d depth %d\n",
			e->id, eb.path.size(), e->x, e->y, e->target_radius, e->depth);
	perf_timer_end(FUNCNAME);
}
