/*
 * InventoryContent.h:
 *  Represents an interactive view of the inventory for the side bar
 */

#ifndef INVENTORYCONTENT_H_
#define INVENTORYCONTENT_H_

#include "SidebarContent.h"

class InventoryContent: public SidebarContent {
public:
	InventoryContent(const BBox& bbox) :
			SidebarContent(bbox), slot_selected(-1) {
	}
	virtual const char* name() {
		return "Inventory";
	}
	virtual void draw(GameState* gs) const;

	virtual int amount_of_pages(GameState* gs);

	virtual bool handle_io(GameState* gs, ActionQueue& queued_actions);

	void reset_slot_selected() {
		slot_selected = -1;
	}
private:
	int slot_selected;
};

#endif /* INVENTORYCONTENT_H_ */
