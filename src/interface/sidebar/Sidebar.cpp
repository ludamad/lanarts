/*
 * Sidebar.cpp:
 *  Handles drawing & state of side bar
 */

#include <ldraw/draw.h>
#include <ldraw/Font.h>
#include <ldraw/DrawOptions.h>

#include "draw/colour_constants.h"
#include "draw/draw_statbar.h"

#include "draw/draw_sprite.h"

#include "gamestate/GameState.h"

#include "objects/PlayerInst.h"

#include "Sidebar.h"

static void draw_statbar_with_text(GameState* gs, const BBox& bbox, int statmin,
		int statmax, Colour statcol, Colour backcol,
		Colour textcol = Colour(0, 0, 0)) {
	draw_statbar(bbox, float(statmin) / statmax, statcol, backcol);

	using namespace ldraw;
	const Font& font = gs->font();
	font.drawf(DrawOptions(CENTER, textcol), bbox.center(), "%d/%d", statmin,
			statmax);
}

/* Helper method for drawing stat bars */
static void draw_player_statbars(GameState* gs, PlayerInst* player, int x,
		int y) {
	ClassStats& class_stats = player->class_stats();
	CoreStats& core = player->effective_stats().core;

	BBox bbox(x, y, x + 100, y + 10);
	if (player->is_ghost()) {
            gs->font().draw(COL_WHITE, Pos(bbox.x1 + 32, bbox.y1 + 2), "GHOST");
        } else {
            draw_statbar_with_text(gs, bbox, core.hp, core.max_hp, COL_GREEN, COL_RED);
	}

	bbox = bbox.translated(0, 15);
	draw_statbar_with_text(gs, bbox, core.mp, core.max_mp, COL_BLUE,
			Colour(200, 200, 200));

	bbox = bbox.translated(0, 15);
	draw_statbar_with_text(gs, bbox, class_stats.xp, class_stats.xpneeded,
			Colour(255, 215, 11), Colour(169, 143, 100));

	float ratio = player->rest_cooldown() / float(REST_COOLDOWN);
	Colour col(200 * ratio, 200 * (1.0f - ratio), 0);
	ldraw::draw_rectangle(col, BBox(x, y + 45, x + 100, y + 55));

	if (player->rest_cooldown() == 0) {
		gs->font().draw(COL_BLACK, Pos(x + 25, y + 44), "can rest");
	}
}

/* Helper method for drawing basic stat information*/
static void draw_player_base_stats(GameState* gs, PlayerInst* player_inst,
		int x, int y, int width) {
	ClassStats& class_stats = player_inst->class_stats();
	CoreStats& core = player_inst->effective_stats().core;
	DerivedStats& physical = player_inst->effective_stats().physical;
	DerivedStats& magical = player_inst->effective_stats().magic;

	int x_interval = width / 2;
	int y_interval = 16;

	gs->font().drawf(ldraw::DrawOptions(COL_WHITE).origin(ldraw::CENTER), Pos(x - 10 + x_interval, 15), "%s",
			gs->get_level()->label().c_str());

//	y += y_interval;

	Pos p1(x, y), p2(x + x_interval, y);
	gs->font().drawf(COL_PALE_RED, Pos(x, y), "Kills %d",
			player_inst->score_stats().kills);

	if (gs->game_settings().regen_on_death) {
		gs->font().drawf(COL_PALE_RED, Pos(x + x_interval, y), "Deaths %d",
				player_inst->score_stats().deaths);
	} else {
		//gs->font().draw(COL_PALE_BLUE, Pos(x + x_interval, y), "Hardcore");
		gs->font().drawf(COL_PALE_YELLOW, Pos(x + x_interval, y), "Level %d", class_stats.xplevel);
	}

	p1.y += y_interval;
	p2.y += y_interval;

	gs->font().drawf(COL_WHITE, p1, "%s", gs->get_level()->label().c_str());
	gs->font().drawf(COL_GOLD, p2, "Gold %d", player_inst->gold(gs));

	p1.y += y_interval;
	p2.y += y_interval;

	gs->font().drawf(COL_PALE_GREEN, p1, "Strength %d", core.strength);
	gs->font().drawf(COL_PALE_BLUE, p2, "Magic %d", core.magic);

	p1.y += y_interval;
	p2.y += y_interval;

	gs->font().drawf(COL_PALE_GREEN, p1, "Defence %d", (int)round(physical.resistance));
	gs->font().drawf(COL_PALE_BLUE, p2, "Will %d", (int)round(magical.resistance));

	p1.y += y_interval;
	p2.y += y_interval; ;

	p1.y += y_interval;
	p2.y += y_interval;

	// Draw hashes if in network debug mode
	if (gs->game_settings().network_debug_mode) {
		// Draw level hash

		gs->font().drawf(COL_MUTED_GREEN, p1, "Hash 0x%X",
				gs->get_level()->game_inst_set().hash());

		p1.y += y_interval;

		// Draw player hashes
		for (int i = 0; i < gs->player_data().all_players().size(); i++) {
			unsigned int hash =
					gs->player_data().all_players()[i].player()->integrity_hash();
			gs->font().drawf(COL_MUTED_GREEN, p1, "P%d 0x%X", i + 1, hash);
			p1.y += y_interval;
		}

		// Draw monster hashes (note, takes up a lot of screen space)
		for (int i = 0; i < gs->monster_controller().monster_ids().size();
				i++) {
			GameInst* inst = gs->get_instance(
					gs->monster_controller().monster_ids()[i]);
			if (inst) {
				gs->font().drawf(COL_MUTED_GREEN, Pos(0, i * y_interval),
						"M%d 0x%X", i + 1, inst->integrity_hash());
			}
		}
	}
}

static BBox content_area_box(const BBox& sidebar_box) {
	const int CONTENT_AREA_Y = 342;
	const int CONTENT_ROWS = 8;
	int sx = sidebar_box.x1, sy = sidebar_box.y1 + CONTENT_AREA_Y;
	int ex = sidebar_box.x2, ey = sy + CONTENT_ROWS * TILE_SIZE;
	return BBox(sx, sy, ex, ey);
}

static Pos minimap_center(const BBox& sidebar_bounds) {
	return sidebar_bounds.left_top() + Pos(74, 158);
}

Sidebar::Sidebar(const BBox& sidebar_bounds) :
		sidebar_bounds(sidebar_bounds), minimap(minimap_center(sidebar_bounds)), navigator(
				sidebar_bounds, content_area_box(sidebar_bounds)) {
}

void Sidebar::draw(GameState* gs) {
	perf_timer_begin(FUNCNAME);
	const int STATBAR_OFFSET_X = 32, STATBAR_OFFSET_Y = 32;

	PlayerInst* p = gs->local_player();

	minimap.draw(gs);
	draw_player_base_stats(gs, p, sidebar_bounds.x1 + 10,
			sidebar_bounds.y1 + 237, sidebar_bounds.width());
	draw_player_statbars(gs, p, sidebar_bounds.x1 + STATBAR_OFFSET_X,
			sidebar_bounds.y1 + STATBAR_OFFSET_Y);
	navigator.draw(gs);
	perf_timer_end(FUNCNAME);
}

void Sidebar::step(GameState* gs) {
}

bool Sidebar::handle_io(GameState* gs, ActionQueue& queued_actions) {
	if (navigator.handle_io(gs, queued_actions)) {
		return true;
	}
	return false;
}

