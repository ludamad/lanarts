/*
 * SidebarNavigator.cpp:
 *  Handles navigation of content in the side bar
 */

#include "../../../data/sprite_data.h"

#include "SidebarContent.h"
#include "InventoryContent.h"
#include "SpellsContent.h"
#include "SidebarNavigator.h"

NavigationSprites::NavigationSprites() :
		left_arrow(get_sprite_by_name("left_arrow")), right_arrow(
				get_sprite_by_name("right_arrow")), inventory_icon(
				get_sprite_by_name("inventory_icon")), stats_icon(
				get_sprite_by_name("stats_icon")), spells_icon(
				get_sprite_by_name("spells_icon")) {
}

SidebarNavigator::SidebarNavigator(const BBox& sidebar_bounds,
		const BBox& main_content_bounds) :
		side_bar(sidebar_bounds), main_content(main_content_bounds) {
	viewtype = INVENTORY;
	inventory = new InventoryContent(main_content_bounds);
	spells = new SpellsContent(main_content_bounds);
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

bool SidebarNavigator::handle_io(GameState* gs, ActionQueue& queued_actions) {
	return current_content()->handle_io(gs, queued_actions);
}

SidebarContent* SidebarNavigator::current_content() {
	switch (viewtype) {
	case INVENTORY:
		return inventory;
		break;
	case SPELLS:
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
