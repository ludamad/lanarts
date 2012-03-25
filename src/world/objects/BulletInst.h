#ifndef BULLETINST_H_
#define BULLETINST_H_

#include "GameInst.h"
#include "../../gamestats/Stats.h"

class BulletInst: public GameInst {
	enum {RADIUS = 5};
public:
	BulletInst(obj_id originator, Attack& attack, int x, int y, int tx, int ty, bool bounce = false);
	~BulletInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
private:
	Attack attack;
	int range_left;
	int origin_id;
	float rx, ry, vx, vy;
	bool bounce;
};

#endif /* BULLETINST_H_ */
