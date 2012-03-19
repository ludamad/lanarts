#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "GameInst.h"
#include "../../combat_logic/Stats.h"

struct EnemyBehaviour {
	enum Action {
		WANDERING,
		CHASING_PLAYER
	};
	Action current_action;
	float speed, vx, vy;
	EnemyBehaviour(float speed) :
		current_action(WANDERING), speed(speed), vx(0), vy(0){
	}
};

class EnemyInst : public GameInst {
	enum {RADIUS = 14};
public:
	EnemyInst(int x, int y) :
		GameInst(x,y, RADIUS), eb(1.0f), rx(x), ry(y), stat(100,100, 5) {
	}
	virtual ~EnemyInst();
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	void attack(GameInst* inst);
	Stats& stats() { return stat; }
	EnemyBehaviour& behaviour() { return eb; }
protected:
	EnemyBehaviour eb;
	float rx, ry;
    Stats stat;
};


#endif /* ENEMYINST_H_ */
