#ifndef ENEMYBEHAVIOUR_H_
#define ENEMYBEHAVIOUR_H_

#include <vector>

#include "../../util/game_basic_structs.h"

#include "GameInst.h"

struct EnemyCooldowns {
	EnemyCooldowns() :
			successful_hit_timeout(0), damage_taken_timer(0) {

	}
	//XXX: This attempts to randomize enemy movement
	//If they have been hit but have not hit anything in some time
	int successful_hit_timeout;
	int damage_taken_timer;

	void step() {
		cooldown_step(successful_hit_timeout);
		cooldown_step(damage_taken_timer);
	}
};

struct EnemyBehaviour {
	enum {
		RANDOM_WALK_COOLDOWN = 3000
	};
	enum Action {
		INACTIVE = 0, FOLLOWING_PATH = 1, CHASING_PLAYER = 2
	};

	std::vector<Pos> path;
	int current_node;
	int path_cooldown;

	float force_x, force_y;
	Action current_action;
	int action_timeout;
	int simulation_id;

	EnemyCooldowns cooldowns;

	Pos path_start;
	int path_steps;
	EnemyBehaviour() :
			current_node(0), path_cooldown(0), force_x(0), force_y(0), current_action(
					INACTIVE), action_timeout(0), simulation_id(0), path_start(
					0, 0), path_steps(0) {
	}
	void step() {
		if (action_timeout) {
			action_timeout--;
			if (action_timeout == 0) {
				current_action = INACTIVE;
			}
		}
	}
};

#endif /* ENEMYBEHAVIOUR_H_ */
