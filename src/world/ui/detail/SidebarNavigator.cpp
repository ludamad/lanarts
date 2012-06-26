/*
 * SidebarNavigator.cpp:
 *  Handles navigation of content in the side bar
 */

#include "InventoryContent.h"
#include "SpellsContent.h"
#include "SidebarNavigator.h"

SidebarNavigator::SidebarNavigator(const BBox& side_bar,
		const BBox& main_content) :
		side_bar(side_bar), main_content(main_content) {
	inventory = new InventoryContent(main_content);
	spells = new SpellsContent(main_content);
	shop = NULL;
}

SidebarNavigator::~SidebarNavigator() {
	delete inventory;
	delete spells;
	delete shop;
}

void SidebarNavigator::draw(GameState* gs) {
	current_content()->draw(gs);
}

void SidebarNavigator::step(GameState* gs) {
	current_content()->step(gs);
}

SidebarContent* SidebarNavigator::current_content() {
	switch (viewtype) {
	case INVENTORY:
		return inventory;
		break;
	case STATS:
		return spells;
		break;
	case SHOP:
		return shop;
		break;
	}
	/*Should never happen*/
	LANARTS_ASSERT(false);
	return NULL;
}

