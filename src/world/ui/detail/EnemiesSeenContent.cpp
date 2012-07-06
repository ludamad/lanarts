/*
 * EnemiesSeenContent.h:
 *  Displays all the enemies players have seen, allowing them to see descriptions.
 */
#include "EnemiesSeenContent.h"

#include "../../../data/sprite_data.h"

#include "../../../util/colour_constants.h"
#include "../../../util/content_draw_util.h"

#include "../../GameState.h"
#include "../../controllers/MonsterController.h"

const int ENEMIES_PER_PAGE = 40;

static void draw_sprite_in_box(sprite_id sprite, const BBox& slotbox) {
	BBox section(0, 0, slotbox.width(), slotbox.height());
	SpriteEntry& spr_entry = game_sprite_data.at(sprite);
	GLimage& img = spr_entry.img();
	gl_draw_image_section(img, section, slotbox.x1, slotbox.y1);
}

static void draw_enemies_seen(GameState* gs, EnemiesSeen& es, const BBox& bbox,
		int min_slot, int max_slot) {
	EnemiesSeen::iterator it = es.begin();
	for (int i = 0; i < min_slot; i++) {
		++it;
	}

	int mx = gs->mouse_x(), my = gs->mouse_y();
	int slot = min_slot;
	for (int y = bbox.y1; y < bbox.y2; y += TILE_SIZE) {
		for (int x = bbox.x1; x < bbox.x2; x += TILE_SIZE) {
			if (slot >= max_slot)
				break;

			EnemyEntry& eentry = game_enemy_data.at(*it);

			BBox slotbox(x, y, x + TILE_SIZE, y + TILE_SIZE);
			Colour outline(COL_UNFILLED_OUTLINE);
			if (it != es.end()) {
				outline = COL_FILLED_OUTLINE;
				if (slotbox.contains(mx, my)) {
					outline = COL_PALE_YELLOW;
					draw_console_enemy_description(gs, eentry);
				}
				draw_sprite_in_box(eentry.enemy_sprite, slotbox);
				++it;
			}
			//draw rectangle over edges

			gl_draw_rectangle_outline(slotbox, outline);
			slot++;
		}
	}

}

void EnemiesSeenContent::draw(GameState* gs) const {
	EnemiesSeen& es = gs->enemies_seen();
	draw_enemies_seen(gs, es, bbox, 0, 40);
}

int EnemiesSeenContent::amount_of_pages(GameState* gs) {
	return gs->enemies_seen().amount() / ENEMIES_PER_PAGE;
}

bool EnemiesSeenContent::handle_io(GameState *gs,
		ActionQueue & queued_actions) {
	return false;
}

EnemiesSeenContent::~EnemiesSeenContent() {
}

