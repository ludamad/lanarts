/*
 * ConfigContent.h:
 *  Visual toggle of configuration options in side bar
 */

#ifndef CONFIGCONTENT_H_
#define CONFIGCONTENT_H_

#include "SidebarContent.h"

class ConfigContent: public SidebarContent {
public:
	ConfigContent(const BBox& bbox) :
			SidebarContent(bbox) {

	}

	virtual const char* name() {
		return "Configuration";
	}

	virtual void draw(GameState* gs) const;

	virtual bool handle_io(GameState* gs, ActionQueue& queued_actions);
};

#endif /* CONFIGCONTENT_H_ */
