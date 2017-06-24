/*
 * InventoryContent.h:
 *  Represents an interactive view of the inventory for the side bar
 */

#ifndef INVENTORYCONTENT_H_
#define INVENTORYCONTENT_H_

#include "SidebarContent.h"
#include <luawrap/LuaValue.h>

class InventoryContent: public SidebarContent {
public:
	InventoryContent(const BBox& bbox) :
			SidebarContent(bbox) {
	}
	virtual const char* name() {
		return "Inventory";
	}
	void ensure_init(GameState* gs) const;
	virtual void draw(GameState* gs) const;

	virtual int amount_of_pages(GameState* gs);
    void reset_slot_selected() {
        slot_selected = -1;
    }

	virtual bool handle_io(GameState* gs, ActionQueue& queued_actions);
private:
	mutable LuaValue handler; // Initialized upon first use
    int slot_selected;
};

#endif /* INVENTORYCONTENT_H_ */
