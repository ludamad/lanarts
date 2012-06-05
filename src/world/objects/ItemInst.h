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
	enum {RADIUS = 10, DEPTH = 100};
	ItemInst(item_id type, int x, int y, int quantity = 1, obj_id dropped_by = 0) :
		GameInst(x,y, RADIUS, false, DEPTH), type(type),
		quantity(quantity), dropped_by(dropped_by){}
	virtual ~ItemInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual ItemInst* clone() const;

	item_id item_type(){ return type; }
	int& item_quantity() { return quantity; }
	obj_id last_held_by() { return dropped_by; }
private:
	item_id type;
	int quantity;
	obj_id dropped_by;
};

#endif /* ITEMINST_H_ */
