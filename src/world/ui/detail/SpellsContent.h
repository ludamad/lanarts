/*
 * SpellsContent.h:
 *  Draws known spells in a grid, for the side bar
 */

#ifndef SPELLSCONTENT_H_
#define SPELLSCONTENT_H_

#include "SidebarContent.h"

class SpellsContent: public SidebarContent {
public:
	SpellsContent(const BBox& bbox) :
			SidebarContent(bbox) {
	}
	virtual void draw(GameState* gs) const;

	virtual int amount_of_pages(GameState* gs);
};

#endif /* SPELLSCONTENT_H_ */
