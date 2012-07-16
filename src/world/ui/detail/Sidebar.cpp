/*
 * Sidebar.cpp:
 *  Handles drawing & state of side bar
 */

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

#include "../../../display/display.h"

#include "../../../util/colour_constants.h"

#include "Sidebar.h"

static void draw_statbar_with_text(GameState* gs, const BBox& bbox, int statmin,
		int statmax, Colour statcol, Colour backcol,
		Colour textcol = Colour(0, 0, 0)) {
	gl_draw_statbar(bbox, statmin, statmax, statcol, backcol);
	gl_printf_centered(gs->primary_font(), textcol, bbox.center_x(),
			bbox.center_y(), "%d/%d", statmin, statmax);

}

/* Helper method for drawing stat bars */
static void draw_player_statbars(GameState* gs, PlayerInst* player, int x,
		int y) {
	ClassStats& class_stats = player->class_stats();
	CoreStats& core = player->effective_stats().core;

	BBox bbox(x, y, x + 100, y + 10);
	draw_statbar_with_text(gs, bbox, core.hp, core.max_hp, COL_GREEN, COL_RED);

	bbox = bbox.translated(0, 15);
	draw_statbar_with_text(gs, bbox, core.mp, core.max_mp, COL_BLUE,
			Colour(200, 200, 200));

	bbox = bbox.translated(0, 15);
	draw_statbar_with_text(gs, bbox, class_stats.xp, class_stats.xpneeded,
			Colour(255, 215, 11), Colour(169, 143, 100));

	float ratio = player->rest_cooldown() / float(REST_COOLDOWN);
	Colour col(200 * ratio, 200 * (1.0f - ratio), 0);
	gl_draw_rectangle(x, y + 45, 100, 10, col);

	if (player->rest_cooldown() == 0)
		gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 25, y + 44,
				"can rest", player->rest_cooldown() * 100 / REST_COOLDOWN);
}

/* Helper method for drawing basic stat information*/
static void draw_player_base_stats(GameState* gs, PlayerInst* player_inst,
		int x, int y, int width) {
	ClassStats& class_stats = player_inst->class_stats();
	CoreStats& core = player_inst->effective_stats().core;

	int x_interval = width / 2;
	int y_interval = 15;

	gl_printf_centered(gs->primary_font(), COL_GOLD, x - 10 + x_interval, 15,
			"Level %d", class_stats.xplevel);
//	y += y_interval;

	gl_printf(gs->primary_font(), COL_GOLD, x, y, "Kills %d",
			player_inst->number_of_kills());
	gl_printf(gs->primary_font(), COL_GOLD, x + x_interval, y, "Deaths %d",
			player_inst->number_of_deaths());
	y += y_interval;

	gl_printf(gs->primary_font(), COL_GOLD, x, y, "Floor %d",
			gs->get_level()->level_number);
	gl_printf(gs->primary_font(), COL_GOLD, x + x_interval, y, "Gold %d",
			player_inst->gold());
	y += y_interval;

	gl_printf(gs->primary_font(), COL_GOLD, x, y, "Strength %d", core.strength);
	gl_printf(gs->primary_font(), COL_GOLD, x + x_interval, y, "Magic %d",
			core.magic);
	y += y_interval;

	gl_printf(gs->primary_font(), COL_GOLD, x, y, "Defence %d", core.defence);
	gl_printf(gs->primary_font(), COL_GOLD, x + x_interval, y, "Will %d",
			core.willpower);
	y += y_interval;
}

static BBox content_area_box(const BBox& sidebar_box) {
	const int CONTENT_AREA_Y = 342;
	const int CONTENT_ROWS = 8;
	int sx = sidebar_box.x1, sy = sidebar_box.y1 + CONTENT_AREA_Y;
	int ex = sidebar_box.x2, ey = sy + CONTENT_ROWS * TILE_SIZE;
	return BBox(sx, sy, ex, ey);
}

static BBox minimap_bounds(const BBox& sidebar_bounds) {
	int minimap_relposx = 20, minimap_relposy = 64 + 45;
	int minimap_w = 128, minimap_h = 128;
	int sx = sidebar_bounds.x1 + minimap_relposx;
	int sy = sidebar_bounds.y1 + minimap_relposy;
	return BBox(sx, sy, sx + minimap_w, sy + minimap_h);
}

Sidebar::Sidebar(const BBox& sidebar_bounds) :
		sidebar_bounds(sidebar_bounds), minimap(minimap_bounds(sidebar_bounds)), navigator(
				sidebar_bounds, content_area_box(sidebar_bounds)) {
}

void Sidebar::draw(GameState* gs) {
	const int STATBAR_OFFSET_X = 32, STATBAR_OFFSET_Y = 32;

	PlayerInst* p = gs->local_player();

	minimap.draw(gs, 1.0f);
	draw_player_base_stats(gs, p, sidebar_bounds.x1 + 10,
			sidebar_bounds.y1 + 237, sidebar_bounds.width());
	draw_player_statbars(gs, p, sidebar_bounds.x1 + STATBAR_OFFSET_X,
			sidebar_bounds.y1 + STATBAR_OFFSET_Y);
	navigator.draw(gs);
}

void Sidebar::step(GameState* gs) {
}

bool Sidebar::handle_io(GameState* gs, ActionQueue& queued_actions) {
	if (navigator.handle_io(gs, queued_actions)) {
		return true;
	}
	return false;
}

