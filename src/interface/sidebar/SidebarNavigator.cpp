/*
 * SidebarNavigator.cpp:
 *  Handles navigation of content in the side bar
 */

#include <ldraw/draw.h>
#include <ldraw/Font.h>
#include <ldraw/DrawOptions.h>

#include "draw/colour_constants.h"

#include "draw/draw_sprite.h"

#include "draw/SpriteEntry.h"

#include "gamestate/GameState.h"

#include "ConfigContent.h"
#include "EnemiesSeenContent.h"
#include "EquipmentContent.h"
#include "InventoryContent.h"
#include "SidebarContent.h"
#include "SidebarNavigator.h"
#include "SpellsContent.h"

SidebarNavigator::NavigationOption::NavigationOption(const std::string& icon,
		std::shared_ptr<SidebarContent> content, const BBox& icon_bbox) :
		iconsprite(icon), content(content), icon_bbox(icon_bbox) {
}

SidebarNavigator::NavigationOption::~NavigationOption() {
}

void SidebarNavigator::NavigationOption::draw_icon(GameState* gs,
		bool selected) {
	Colour col;
	if (selected) {
		col = COL_SELECTED_VIEWICON;
	} else if (icon_bbox.contains(gs->mouse_x(), gs->mouse_y())) {
		col = COL_HOVER_VIEWICON;
	}
	draw_sprite(res::sprite_id(iconsprite.c_str()), icon_bbox.x1, icon_bbox.y1, col);
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
				INVENTORY), content_overlay(NULL), inventory("inventory_icon",
				std::make_shared<InventoryContent>(main_content_bounds),
				icon_bounds(main_content_bounds, 0, NUM_ICONS_ROW_1)), equipment(
				"equipment_icon", std::make_shared<EquipmentContent>(main_content_bounds),
				icon_bounds(main_content_bounds, 1, NUM_ICONS_ROW_1)), spells(
				"spells_icon", std::make_shared<SpellsContent>(main_content_bounds),
				icon_bounds(main_content_bounds, 2, NUM_ICONS_ROW_1)), enemies(
				"enemies_icon", std::make_shared<EnemiesSeenContent>(main_content_bounds),
				icon_bounds(main_content_bounds, 0, NUM_ICONS_ROW_2, 1)), config(
				"config_icon", std::make_shared<ConfigContent>(main_content_bounds),
				icon_bounds(main_content_bounds, 1, NUM_ICONS_ROW_2, 1)) {
}

SidebarNavigator::~SidebarNavigator() {
}

void SidebarNavigator::draw(GameState* gs) {
	if (!content_overlay) {
		inventory.draw_icon(gs);
		spells.draw_icon(gs);
		enemies.draw_icon(gs);
		equipment.draw_icon(gs);
		config.draw_icon(gs);
		current_option().draw_icon(gs, true);
	}

	current_content()->draw(gs);

	using namespace ldraw;
	const Font& font = gs->font();
	font.draw(DrawOptions(CENTER_TOP, COL_FILLED_OUTLINE),
			Pos(main_content.center_x(), main_content.y2 + 3),
			current_content()->name());

	content_overlay = NULL;
}

void SidebarNavigator::step(GameState* gs) {
	current_content()->step(gs);
}

void SidebarNavigator::reset_slot_selected() {
	((InventoryContent*)inventory.content.get())->reset_slot_selected();
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
			|| handle_icon_io(gs, config, CONFIG)
			|| handle_icon_io(gs, enemies, ENEMIES)
			|| handle_icon_io(gs, equipment, EQUIPMENT)) {
		return true;
	}
	return current_content()->handle_io(gs, queued_actions);
}

SidebarContent* SidebarNavigator::current_content() {
	if (!content_overlay) {
		return current_option().content.get();
	} else {
		return content_overlay;
	}
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
