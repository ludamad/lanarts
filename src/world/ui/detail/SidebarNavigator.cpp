/*
 * SidebarNavigator.cpp:
 *  Handles navigation of content in the side bar
 */

#include "../../../data/sprite_data.h"

#include "../../../display/display.h"

#include "../../../util/colour_constants.h"

#include "../../GameState.h"

#include "EquipmentContent.h"
#include "EnemiesSeenContent.h"
#include "InventoryContent.h"
#include "SidebarContent.h"
#include "SpellsContent.h"
#include "SidebarNavigator.h"

NavigationSprites::NavigationSprites() :
		left_arrow(get_sprite_by_name("left_arrow")), right_arrow(
				get_sprite_by_name("right_arrow")) {
}
SidebarNavigator::NavigationOption::NavigationOption(sprite_id icon,
		SidebarContent* content, const BBox& icon_bbox) :
		icon(icon), content(content), icon_bbox(icon_bbox) {
}

SidebarNavigator::NavigationOption::~NavigationOption() {
	delete content;
}

void SidebarNavigator::NavigationOption::draw_icon(GameState* gs,
		bool selected) {
	Colour col;
	if (selected) {
		col = COL_SELECTED_VIEWICON;
	} else if (icon_bbox.contains(gs->mouse_x(), gs->mouse_y())) {
		col = COL_HOVER_VIEWICON;
	}
	gl_draw_sprite(icon, icon_bbox.x1, icon_bbox.y1, col);
}

static BBox icon_bounds(const BBox& main_content_bounds, int icon_n,
		int total_n, int row = 0) {
	int icon_sep = TILE_SIZE + 2;
	int icon_sx = main_content_bounds.center_x() - icon_sep * total_n / 2;
	int icon_sy = main_content_bounds.y2 + (row + 1) * (TILE_SIZE + 2);

	int icon_x = icon_sx + icon_n * icon_sep;

	return BBox(icon_x, icon_sy, icon_x + TILE_SIZE, icon_sy + TILE_SIZE);
}

const int NUM_ICONS_ROW_1 = 3;
const int NUM_ICONS_ROW_2 = 2;

SidebarNavigator::SidebarNavigator(const BBox& sidebar_bounds,
		const BBox& main_content_bounds) :
		side_bar(sidebar_bounds), main_content(main_content_bounds), view(
				INVENTORY), inventory(get_sprite_by_name("inventory_icon"),
				new InventoryContent(main_content_bounds),
				icon_bounds(main_content_bounds, 0, NUM_ICONS_ROW_1)), equipment(
				get_sprite_by_name("equipment_icon"),
				new EquipmentContent(main_content_bounds),
				icon_bounds(main_content_bounds, 1, NUM_ICONS_ROW_1)), spells(
				get_sprite_by_name("spells_icon"),
				new SpellsContent(main_content_bounds),
				icon_bounds(main_content_bounds, 2, NUM_ICONS_ROW_1)), enemies(
				get_sprite_by_name("enemies_icon"),
				new EnemiesSeenContent(main_content_bounds),
				icon_bounds(main_content_bounds, 0, NUM_ICONS_ROW_2, 1)), config(
				get_sprite_by_name("config_icon"),
				new EquipmentContent(main_content_bounds),
				icon_bounds(main_content_bounds, 1, NUM_ICONS_ROW_2, 1)) {
}

SidebarNavigator::~SidebarNavigator() {
}

void SidebarNavigator::draw(GameState* gs) {
	inventory.draw_icon(gs);
	spells.draw_icon(gs);
	enemies.draw_icon(gs);
	equipment.draw_icon(gs);
	config.draw_icon(gs);
	current_option().draw_icon(gs, true);

	current_content()->draw(gs);
	gl_printf_x_centered(gs->primary_font(), COL_FILLED_OUTLINE,
			main_content.center_x(), main_content.y2 + 3,
			current_content()->name());
}

void SidebarNavigator::step(GameState* gs) {
	current_content()->step(gs);
}

void SidebarNavigator::reset_slot_selected() {
	((InventoryContent*)inventory.content)->reset_slot_selected();
}

bool SidebarNavigator::handle_icon_io(GameState* gs, NavigationOption& option,
		view_t value) {
	if (gs->mouse_left_click()
			&& option.icon_bbox.contains(gs->mouse_x(), gs->mouse_y())) {
		view = value;
		return true;
	}
	return false;
}
bool SidebarNavigator::handle_io(GameState* gs, ActionQueue& queued_actions) {
	if (handle_icon_io(gs, inventory, INVENTORY)
			|| handle_icon_io(gs, spells, SPELLS)
//			|| handle_icon_io(gs, config, CONFIG)
			|| handle_icon_io(gs, enemies, ENEMIES)
			|| handle_icon_io(gs, equipment, EQUIPMENT)) {
		return true;
	}
	return current_content()->handle_io(gs, queued_actions);
}

SidebarContent* SidebarNavigator::current_content() {
	return current_option().content;
}

SidebarNavigator::NavigationOption& SidebarNavigator::current_option() {
	switch (view) {
	case INVENTORY:
		return inventory;
	case SPELLS:
		return spells;
	case ENEMIES:
		return enemies;
	case EQUIPMENT:
		return equipment;
	case CONFIG:
		return config;
	}
	/* shouldn't happen */
	LANARTS_ASSERT(false);
	return inventory;
}
