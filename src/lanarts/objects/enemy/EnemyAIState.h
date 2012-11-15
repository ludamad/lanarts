/*
 * EnemyAIState.h:
 *  State information that affects enemy AI decisions
 */

#ifndef ENEMYAISTATE_H_
#define ENEMYAISTATE_H_

#include <cstring>
#include <vector>
#include <common/SerializeBuffer.h>

#include "../../collision_avoidance/CollisionAvoidance.h"

#include "../../lanarts_defines.h"

class GameState;

struct EnemyAIPath {
	std::vector<Pos> nodes;
	Pos start_position;
	int path_steps;
	EnemyAIPath() :
			path_steps(0) {
	}

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);
};

struct EnemyAction {
	enum action_t {
		SLEEPING = 0, WANDERING = 1, FOLLOWING_PATH = 2, FOLLOWING_OBJECT = 3
	};

	action_t current_action;
	EnemyAIPath path;

	EnemyAction() :
			current_action(SLEEPING) {
	}

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);
};

struct EnemyCounters {
	int chase_timeout;
	int successful_hit_timer;
	int damage_taken_timer;
	int random_walk_timer;
	EnemyCounters() {
		memset(this, 0, sizeof(EnemyCounters));
	}
};

struct EnemyAIState {
	EnemyCounters counters;
	EnemyAction action;

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);
};

#endif /* ENEMYAISTATE_H_ */
