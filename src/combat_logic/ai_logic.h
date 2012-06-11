/*
 * ai_logic.h:
 *  Represents movement and attack decisions for an entity
 */

#ifndef AI_LOGIC_H_
#define AI_LOGIC_H_

class GameState;
class CombatGameInst;

struct AIPathing {
	std::vector<Pos> path;
	int current_node;
	int path_cooldown;

	Pos path_start;
	int path_steps;

	void step();

	AIPathing() :
			current_node(0), path_cooldown(0), path_start(0, 0), path_steps(0) {

	}
};

struct AIBehaviour {
	enum {
		RANDOM_WALK_COOLDOWN = 3000
	};
	enum Action {
		INACTIVE = 0, FOLLOWING_PATH = 1, CHASING_PLAYER = 2
	};

	float force_x, force_y;
	Action current_action;
	int action_timeout;
	int simulation_id;
	float speed, vx, vy;

	AIBehaviour(float speed) :
			force_x(0), force_y(0), current_action(INACTIVE), action_timeout(0), simulation_id(
					0), speed(speed), vx(0), vy(0) {
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

void ai_decisions(GameState* gs, CombatGameInst* inst);

#endif /* AI_LOGIC_H_ */
