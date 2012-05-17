#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "GameInst.h"
#include "../../gamestats/Stats.h"
#include "../../data/enemy_data.h"
#include <vector>
#include "../../procedural/GeneratedLevel.h"

struct EnemyBehaviour {
	enum {
		RANDOM_WALK_COOLDOWN = 3000
	};
	enum Action {
		INACTIVE,
		FOLLOWING_PATH,
		CHASING_PLAYER
	};

	std::vector<Pos> path;
	int current_node;
	int path_cooldown;

	float force_x, force_y;
	Action current_action;
	int action_timeout;
	int simulation_id;
	float speed, vx, vy;

	Pos path_start;
	int path_steps;
	EnemyBehaviour(float speed) :
		current_node(0), path_cooldown(0),
		force_x(0), force_y(0),
		current_action(INACTIVE), action_timeout(0), simulation_id(0),
		speed(speed), vx(0), vy(0),
		path_start(0,0), path_steps(1){
	}
	void step(){
		if (action_timeout){
			action_timeout--;
			if (action_timeout == 0){
				current_action = INACTIVE;
			}
		}
	}
};

class EnemyInst : public GameInst {
public:
	EnemyInst(int enemytype, int x, int y);
	virtual ~EnemyInst();
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	void attack(GameState* gs, GameInst* inst, bool ranged);
	Stats& stats() { return stat; }
	EnemyBehaviour& behaviour() { return eb; }
	EnemyType* etype();
	int xpworth(){
		return xpgain;
	}
	int& last_seen(){
		return last_seen_counter;
	}
	bool hurt(GameState* gs, int hp);
public:
	float rx, ry;
protected:
	int last_seen_counter;
	EnemyBehaviour eb;
	int enemytype;
	int xpgain;
    Stats stat;
};


#endif /* ENEMYINST_H_ */
