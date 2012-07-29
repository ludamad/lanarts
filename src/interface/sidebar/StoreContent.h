/*
 * StoreContent.h:
 *  Represents an interactive view of a store for the side bar
 */

#ifndef STORECONTENT_H_
#define STORECONTENT_H_

#include "SidebarContent.h"

class StoreContent: public SidebarContent {
public:
	StoreContent(const BBox& bbox) :
			SidebarContent(bbox), slot_selected(-1) {
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
	int storeid;
	int slot_selected;
};

#endif /* STORECONTENT_H_ */
