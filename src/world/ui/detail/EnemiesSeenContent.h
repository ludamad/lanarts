/*
 * EnemiesSeenContent.h:
 *  Displays all the enemies players have seen, allowing them to see descriptions.
 */

#ifndef ENEMIESSEENCONTENT_H_
#define ENEMIESSEENCONTENT_H_

#include "SidebarContent.h"

class EnemiesSeenContent: public SidebarContent {
public:
	EnemiesSeenContent(const BBox& bbox) :
			SidebarContent(bbox) {
	}

	virtual const char* name() {
		return "Enemies Seen";
	}

	virtual void draw(GameState* gs) const;

	virtual int amount_of_pages(GameState* gs);

	virtual bool handle_io(GameState* gs, ActionQueue& queued_actions);

	virtual ~EnemiesSeenContent();
};

#endif /* ENEMIESSEENCONTENT_H_ */
