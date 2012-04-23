/*
 * ItemInst.h
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */

#ifndef ITEMINST_H_
#define ITEMINST_H_

#include "GameInst.h"

class ItemInst: public GameInst {
public:
	enum {RADIUS = 10};
	ItemInst(int type, int x, int y, obj_id dropped_by = 0) :
		GameInst(x,y, RADIUS, false), type(type), dropped_by(dropped_by){}
	virtual ~ItemInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	int item_type(){ return type; }
	obj_id last_held_by() { return dropped_by; }
private:
	int type;
	obj_id dropped_by;
};

#endif /* ITEMINST_H_ */
