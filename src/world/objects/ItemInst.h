/*
 * ItemInst.h:
 *  Represents an item on the floor
 */

#ifndef ITEMINST_H_
#define ITEMINST_H_

#include "../../gamestats/items.h"

#include "GameInst.h"

class ItemInst: public GameInst {
public:
	enum {
		RADIUS = 15, DEPTH = 100
	};
	ItemInst(const Item& item, int x, int y, int quantity = 1,
			obj_id dropped_by = 0, bool pickup_by_dropper = false) :
			GameInst(x, y, RADIUS, false, DEPTH), item(item), quantity(
					quantity), dropped_by(dropped_by), pickup_by_dropper(
					pickup_by_dropper) {
	}
	virtual ~ItemInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual ItemInst* clone() const;

	Item& item_type() {
		return item;
	}
	int& item_quantity() {
		return quantity;
	}
	obj_id last_held_by() {
		return dropped_by;
	}
	bool autopickup_held() {
		return pickup_by_dropper;
	}
private:
	Item item;
	int quantity;
	obj_id dropped_by;
	bool pickup_by_dropper;
};

#endif /* ITEMINST_H_ */
