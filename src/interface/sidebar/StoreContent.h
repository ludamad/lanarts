/*
 * StoreContent.h:
 *  Represents an interactive view of a store for the side bar
 */

#ifndef STORECONTENT_H_
#define STORECONTENT_H_

#include "SidebarContent.h"

class StoreInst;

class StoreContent: public SidebarContent {
public:
	StoreContent(StoreInst* store_object = NULL, const BBox& bbox = BBox()) :
			SidebarContent(bbox), store_object(store_object), slot_selected(-1) {
	}
	void init(StoreInst* store_object, const BBox& bbox) {
		this->store_object = store_object;
		this->bbox = bbox;
	}
	virtual const char* name() {
		return "Store";
	}
	virtual void draw(GameState* gs) const;

	virtual int amount_of_pages(GameState* gs) {
		return 1;
	}

	virtual bool handle_io(GameState* gs, ActionQueue& queued_actions);

	void reset_slot_selected() {
		slot_selected = -1;
	}

private:
	StoreInst* store_object;
	int slot_selected;
};

#endif /* STORECONTENT_H_ */
