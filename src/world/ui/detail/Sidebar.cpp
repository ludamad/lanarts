/*
 * Sidebar.cpp:
 *  Handles drawing & state of side bar
 */

#include "../../GameState.h"

#include "../../objects/PlayerInst.h"

#include "Sidebar.h"

/* Helper method for drawing stat bars */
static void draw_player_statbars(GameState* gs, PlayerInst* player, int x,
		int y) {
	ClassStats& class_stats = player->class_stats();
	CoreStats& core = player->effective_stats().core;

	gl_draw_statbar(x, y, 100, 10, core.hp, core.max_hp);
	gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 30, y, "%d/%d", core.hp,
			core.max_hp);

	gl_draw_statbar(x, y + 15, 100, 10, core.mp, core.max_mp, Colour(0, 0, 255),
			Colour(200, 200, 200));

	gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 30, y + 15, "%d/%d",
			core.mp, core.max_mp);
	gl_draw_statbar(x, y + 30, 100, 10, class_stats.xp, class_stats.xpneeded,
			Colour(255, 215, 11), Colour(169, 143, 100));

	gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 30, y + 30, "%d/%d",
			class_stats.xp, class_stats.xpneeded);

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

	int midx = x + width / 2;

	gl_printf(gs->primary_font(), Colour(255, 215, 11), midx - 15, 10,
			"Level %d", class_stats.xplevel);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), midx - 40,
			64 + 45 + 128, "Floor %d", gs->get_level()->level_number);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), midx - 40,
			64 + 45 + 128 + 15, "Gold %d", player_inst->gold());
	gl_printf(gs->primary_font(), Colour(255, 215, 11), midx - 50,
			64 + 45 + 128 + 30, "Strength %d", core.strength);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), midx - 50,
			64 + 45 + 128 + 45, "Magic    %d", core.magic);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), midx - 50,
			64 + 45 + 128 + 60, "Defence  %d", core.defence);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), midx - 50,
			64 + 45 + 128 + 75, "Will     %d", core.willpower);
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

void Sidebar::draw(GameState *gs) {
	const int STATBAR_OFFSET_X = 32, STATBAR_OFFSET_Y = 32;

	PlayerInst* p = gs->local_player();

	if (!p) {
		return;
	}

	minimap.draw(gs, 1.0f);
	navigator.draw(gs);
	draw_player_base_stats(gs, p, sidebar_bounds.x1, sidebar_bounds.y1,
			sidebar_bounds.width());
	draw_player_statbars(gs, p, sidebar_bounds.x1 + STATBAR_OFFSET_X,
			sidebar_bounds.y1 + STATBAR_OFFSET_Y);
}

void Sidebar::step(GameState *gs) {
}

