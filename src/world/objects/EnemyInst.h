#ifndef ENEMYINST_H_
#define ENEMYINST_H_

#include "GameInst.h"

class EnemyInst : public GameInst {
	enum {RADIUS = 10};
public:
	EnemyInst(int x, int y) :
		GameInst(x,y, RADIUS), vx(4), vy(4), steps(0) {}
	virtual ~EnemyInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
protected:
	int vx, vy;
    int steps;
};


#endif /* ENEMYINST_H_ */
