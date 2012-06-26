/*
 * SidebarContent.h:
 *  Abstract class that represents content in the side bar
 */

#ifndef SIDEBARCONTENT_H_
#define SIDEBARCONTENT_H_

#include "../../../util/game_basic_structs.h"

class GameState;

class SidebarContent {
public:
	SidebarContent(const BBox& bbox) :
			bbox(bbox) {
	}
	virtual ~SidebarContent() {
	}
	virtual void draw(GameState* gs) const = 0;
	virtual void step(GameState* gs) {
	}
protected:
	BBox bbox;
};

#endif /* SIDEBARCONTENT_H_ */
