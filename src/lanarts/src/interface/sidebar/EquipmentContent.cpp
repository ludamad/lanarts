/*
 * EquipmentContent.cpp:
 *  Represents an interactive view of equipped items for the side bar
 */

#include <ldraw/draw.h>

#include "draw/colour_constants.h"
#include "draw/draw_sprite.h"

#include "draw/SpriteEntry.h"

#include "gamestate/GameState.h"

#include "objects/player/PlayerInst.h"

#include "stats/items/WeaponEntry.h"

#include "stats/items/ItemEntry.h"
#include "stats/items/ProjectileEntry.h"

#include "../console_description_draw.h"

#include "EquipmentContent.h"

EquipmentContent::~EquipmentContent() {
}

static void draw_equipment_slot(GameState* gs, Inventory& inventory,
		itemslot_t ind, BBox bbox, const char* noslot) {
	Colour bbox_col = COL_UNFILLED_OUTLINE;
	if (ind != -1) {
		ItemSlot& itemslot = inventory.get(ind);
		EquipmentEntry& entry = itemslot.equipment_entry();
		bbox_col = COL_FILLED_OUTLINE;
		if (bbox.contains(gs->mouse_pos())) {
			bbox_col = COL_WHITE;
			draw_console_item_description(gs, itemslot.item,
					itemslot.equipment_entry());
		}
		draw_icon_and_name(gs, entry, COL_WHITE, bbox.x1, bbox.y1);
	} else {
		ldraw::draw_rectangle_outline(COL_PALE_YELLOW.alpha(50),
				BBox(bbox.left_top(), Dim(TILE_SIZE, TILE_SIZE)));
		/* Draw item name */
		Pos offset(TILE_SIZE * 1.25, TILE_SIZE / 2);
		gs->font().draw(ldraw::DrawOptions(COL_GRAY).origin(ldraw::LEFT_CENTER),
				bbox.left_top() + offset, noslot);
	}
	ldraw::draw_rectangle_outline(bbox_col, bbox);
}

static void draw_weapon(GameState* gs, EquipmentStats& eqp, const BBox& bbox) {
	Colour bbox_col = COL_FILLED_OUTLINE;
	if (bbox.contains(gs->mouse_pos())) {
		bbox_col = COL_WHITE;
		draw_console_item_description(gs, eqp.weapon(),
				eqp.weapon().weapon_entry());
	}

	draw_icon_and_name(gs, eqp.weapon().weapon_entry(), COL_WHITE, bbox.x1,
			bbox.y1);
	ldraw::draw_rectangle_outline(bbox_col, bbox);
}

struct SlotAndDefault {
	EquipmentEntry::equip_type type;
	const char* empty_msg;

	SlotAndDefault(EquipmentEntry::equip_type type, const char* empty_msg) :
			type(type), empty_msg(empty_msg) {
	}
};

const SlotAndDefault slot_data[] = { SlotAndDefault(EquipmentEntry::PROJECTILE,
		"No Projectile"), SlotAndDefault(EquipmentEntry::BODY_ARMOUR,
		"No Armour"), SlotAndDefault(EquipmentEntry::BOOTS, "No Boots"),
		SlotAndDefault(EquipmentEntry::GLOVES, "No Gloves"), SlotAndDefault(
				EquipmentEntry::HELMET, "No Helmet"), };

const size_t slot_data_n = sizeof(slot_data) / sizeof(SlotAndDefault);

void EquipmentContent::draw(GameState* gs) const {
	PlayerInst* p = gs->local_player();

	EquipmentStats& eqp = p->equipment();

	BBox other_bbox(bbox.x1, bbox.y1, bbox.x2, bbox.y1 + TILE_SIZE);
	ldraw::draw_rectangle_outline(COL_UNFILLED_OUTLINE, bbox);
	draw_weapon(gs, eqp, other_bbox);

	for (int i = 0; i < slot_data_n; i++) {
		other_bbox.translate(0, TILE_SIZE);
		draw_equipment_slot(gs, eqp.inventory,
				eqp.inventory.get_equipped(slot_data[i].type), other_bbox,
				slot_data[i].empty_msg);
	}

	int ring1 = eqp.inventory.get_equipped(EquipmentEntry::RING);
	int ring2 = eqp.inventory.get_equipped(EquipmentEntry::RING, ring1);

	other_bbox.translate(0, TILE_SIZE);
	draw_equipment_slot(gs, eqp.inventory, ring1, other_bbox, "No First Ring");
	// TODO: Refactor so this isn't a (mostly) cut-n-paste of above
	other_bbox.translate(0, TILE_SIZE);
	draw_equipment_slot(gs, eqp.inventory, ring2, other_bbox, "No Second Ring");
}

bool EquipmentContent::handle_io(GameState* gs, ActionQueue& queued_actions) {
	PlayerInst* p = gs->local_player();
	Inventory& inv = p->inventory();
	BBox other_bbox(bbox.x1, bbox.y1, bbox.x2, bbox.y1 + TILE_SIZE);
	// Check if we are de-equipping our weapon
	Pos mouse = gs->mouse_pos();
	if (gs->mouse_right_click() && other_bbox.contains(mouse)) {
		queued_actions.push_back(
				game_action(gs, p, GameAction::DEEQUIP_ITEM,
						EquipmentEntry::WEAPON));
	}

	// Check if we are de-equipping our armour
	for (int i = 0; i < slot_data_n; i++) {
		other_bbox.translate(0, TILE_SIZE);
		if (gs->mouse_right_click() && other_bbox.contains(mouse)) {
			queued_actions.push_back(
					game_action(gs, p, GameAction::DEEQUIP_ITEM,
							slot_data[i].type));
		}
	}

	int ring1 = inv.get_equipped(EquipmentEntry::RING);
	int ring2 = inv.get_equipped(EquipmentEntry::RING, ring1);
	other_bbox.translate(0, TILE_SIZE);
	if (ring1 >= 0 && gs->mouse_right_click() && other_bbox.contains(mouse)) {
		queued_actions.push_back(
				game_action(gs, p, GameAction::USE_ITEM, ring1));
	}
	// TODO: Refactor so this isn't a mostly cut-n-paste of above
	other_bbox.translate(0, TILE_SIZE);
	if (ring2 >= 0 && gs->mouse_right_click() && other_bbox.contains(mouse)) {
		queued_actions.push_back(
				game_action(gs, p, GameAction::USE_ITEM, ring2));
	}
	return false;
}

