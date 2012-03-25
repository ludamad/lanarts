#ifndef BULLETINST_H_
#define BULLETINST_H_

#include <cstdlib>
#include "GameInst.h"
#include "../../gamestats/Stats.h"

class BulletInst: public GameInst {
	enum {RADIUS = 5};
public:
	BulletInst(obj_id originator, Attack& attack, int x, int y, int tx, int ty,
			bool bounce = false, int hits = 1, GameInst* target = NULL);
	~BulletInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	GameInst* hit_target(){return target;}
private:
	Attack attack;
	int range_left;
	int origin_id;
	float rx, ry, vx, vy;
	bool bounce;
	int hits;
	GameInst* target;
};

#endif /* BULLETINST_H_ */
