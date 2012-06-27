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
			page_number(0), bbox(bbox) {
	}
	virtual ~SidebarContent() {
	}
	virtual void draw(GameState* gs) const = 0;
	virtual void step(GameState* gs) {
	}

	void goto_previous_page() {
		page_number--;
	}
	void goto_next_page() {
		page_number--;
	}

	bool has_previous_page(GameState* gs) {
		return page_number > 0;
	}
	bool has_next_page(GameState* gs) {
		return page_number + 1 < amount_of_pages(gs);
	}

	virtual int amount_of_pages(GameState* gs) {
		return 1;
	}

protected:
	int page_number;
	BBox bbox;
};

#endif /* SIDEBARCONTENT_H_ */
