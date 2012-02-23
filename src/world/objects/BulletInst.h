#ifndef BULLETINST_H_
#define BULLETINST_H_

#include "GameInst.h"

class BulletInst: public GameInst {
	enum {RADIUS = 5};
public:
	BulletInst(int x, int y, int tx, int ty);
	~BulletInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
private:
	float rx, ry, vx, vy;
};

#endif /* BULLETINST_H_ */
