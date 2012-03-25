/*
 * BounceShotInst.h
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */

#ifndef BOUNCESHOTINST_H_
#define BOUNCESHOTINST_H_

#include "GameInst.h"
#include "../../gamestats/Stats.h"


class BounceShotInst: public GameInst {
	enum {RADIUS = 5};
public:
	BounceShotInst(obj_id originator, Attack& attack, int x, int y, int tx, int ty);
	virtual ~BounceShotInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
private:
	Attack attack;
	int range_left;
	int origin_id;
	float rx, ry, vx, vy;
};


#endif /* BOUNCESHOTINST_H_ */
