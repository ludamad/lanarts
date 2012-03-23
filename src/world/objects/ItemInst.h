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
	ItemInst(int type, int x, int y) :
		GameInst(x,y, RADIUS, false), type(type){}
	virtual ~ItemInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	int item_type(){ return type; }
private:
	int type;
};

#endif /* ITEMINST_H_ */
