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
	ItemInst(int x, int y) :
		GameInst(x,y, RADIUS){}
	virtual ~ItemInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
};

#endif /* ITEMINST_H_ */
