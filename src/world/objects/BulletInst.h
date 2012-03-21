#ifndef BULLETINST_H_
#define BULLETINST_H_

#include "GameInst.h"

class BulletInst: public GameInst {
	enum {RADIUS = 5};
public:
	BulletInst(obj_id originator, int speed, int range, int x, int y, int tx, int ty);
	~BulletInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
private:
	int speed;
	int range_left;
	int origin_id;
	float rx, ry, vx, vy;
};

#endif /* BULLETINST_H_ */
