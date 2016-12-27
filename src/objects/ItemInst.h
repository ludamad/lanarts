/*
 * ItemInst.h:
 *  Represents an item on the floor
 */

#ifndef ITEMINST_H_
#define ITEMINST_H_

#include "stats/items/items.h"

#include "GameInst.h"

class ItemInst: public GameInst {
public:
	enum {
		RADIUS = 15, DEPTH = 100
	};
	ItemInst(const Item& item, const Pos& xy, obj_id dropped_by = 0,
			bool pickup_by_dropper = false) :
			GameInst(xy.x, xy.y, RADIUS, false, DEPTH), item(item), dropped_by(
					dropped_by), pickup_by_dropper(pickup_by_dropper), was_seen(false) {
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
		return item.amount;
	}
	obj_id last_held_by() {
		return dropped_by;
	}
	bool autopickup_held() {
		return pickup_by_dropper;
	}
	void set_as_seen() {
	    was_seen = true;
	}

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);

private:
	Item item;
	obj_id dropped_by;
	bool pickup_by_dropper, was_seen;
};

#endif /* ITEMINST_H_ */
