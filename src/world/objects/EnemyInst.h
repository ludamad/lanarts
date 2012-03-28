#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "GameInst.h"
#include "../../gamestats/Stats.h"
#include "../../data/enemy_data.h"
#include <vector>
#include "../../procedural/GeneratedLevel.h"

struct EnemyBehaviour {
	enum Action {
		WANDERING,
		CHASING_PLAYER
	};

	std::vector<Pos> path;
	int current_node;

	Action current_action;
	int action_timeout;
	float speed, vx, vy;
	EnemyBehaviour(float speed) :
		current_node(0), current_action(WANDERING), action_timeout(0), speed(speed), vx(0), vy(0){
	}
	void step(){
		if (action_timeout){
			action_timeout--;
			if (action_timeout == 0){
				current_action = WANDERING;
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
