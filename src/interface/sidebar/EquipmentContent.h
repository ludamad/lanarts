/*
 * EquipmentContent.h:
 *  Represents an interactive view of equipped items for the side bar
 */

#ifndef EQUIPMENTCONTENT_H_
#define EQUIPMENTCONTENT_H_

#include "SidebarContent.h"

class EquipmentContent: public SidebarContent {
public:
	EquipmentContent(const BBox& bbox) :
			SidebarContent(bbox) {
	}

	virtual const char* name() {
		return "Equipment";
	}

	virtual void draw(GameState* gs) const;

	virtual int amount_of_pages(GameState* gs);



	virtual bool handle_io(GameState* gs, ActionQueue& queued_actions);

	virtual ~EquipmentContent();
};

#endif /* EQUIPMENTCONTENT_H_ */
