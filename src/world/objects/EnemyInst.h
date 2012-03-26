#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "GameInst.h"
#include "../../gamestats/Stats.h"
#include "../../data/enemy_data.h"

struct EnemyBehaviour {
	enum Action {
		WANDERING,
		CHASING_PLAYER
	};
	Action current_action;
	int action_timeout;
	float speed, vx, vy;
	EnemyBehaviour(float speed) :
		current_action(WANDERING), action_timeout(0), speed(speed), vx(0), vy(0){
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
	EnemyInst(EnemyType* type, int x, int y) :
		GameInst(x,y, type->radius), eb(type->basestats.movespeed), type(type), rx(x), ry(y),
		xpgain(type->xpaward),stat(type->basestats) {
		last_seen_counter = 0;
	}
	virtual ~EnemyInst();
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	void attack(GameState* gs, GameInst* inst, bool ranged);
	Stats& stats() { return stat; }
	EnemyBehaviour& behaviour() { return eb; }
	EnemyType* etype(){
		return type;
	}
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
	EnemyType* type;
	int xpgain;
    Stats stat;
};


#endif /* ENEMYINST_H_ */
