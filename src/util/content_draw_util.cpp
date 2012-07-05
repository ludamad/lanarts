/*
 * content_draw_util.cpp:
 *  Common routines for content descriptions and drawing
 */

#include "../data/armour_data.h"
#include "../data/item_data.h"
#include "../data/spell_data.h"
#include "../data/sprite_data.h"
#include "../data/weapon_data.h"

#include "../display/display.h"

#include "../gamestats/items.h"

#include "../util/colour_constants.h"

#include "../world/GameState.h"
#include "../world/objects/PlayerInst.h"

#include "content_draw_util.h"

const char* projectile_speed_description(int speed) {
	return NULL;
}

const char* monster_speed_description(int speed) {
	return NULL;
}

void draw_stat_text(GameState* gs, int x, int y, const char* prefix,
		Colour prefix_col, int stat, Colour stat_col) {
	Pos p = gl_printf_y_centered(gs->primary_font(), prefix_col, x, y, "%s",
			prefix);
	gl_printf_y_centered(gs->primary_font(), stat_col, p.x + x, y, "%d", stat);
}

static void draw_range_with_prefix(GameState* gs, int x, int y,
		const char* prefix, Colour prefix_col, const Range& r,
		Colour stat_col) {
	Pos p = gl_printf_y_centered(gs->primary_font(), prefix_col, x, y, "%s",
			prefix);
	gl_printf_y_centered(gs->primary_font(), stat_col, p.x + x, y, "%d to %d",
			r.min, r.max);
}

void draw_item_icon_and_name(GameState* gs, ItemEntry& ientry, Colour col,
		int x, int y) {
	gl_draw_sprite(ientry.sprite, x, y);
	gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE,
			COL_PALE_YELLOW.with_alpha(50));
	/* Draw item name */
	gl_printf_y_centered(gs->primary_font(), col, x + TILE_SIZE * 1.25,
			y + TILE_SIZE / 2, "%s", ientry.name.c_str());
}

void draw_spell_icon_and_name(GameState* gs, SpellEntry& spl_entry, Colour col,
		int x, int y) {
	gl_draw_sprite(spl_entry.sprite, x, y);
	gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE,
			COL_PALE_YELLOW.with_alpha(50));
	/* Draw spell name */
	gl_printf_y_centered(gs->primary_font(), col, x + TILE_SIZE * 1.25,
			y + TILE_SIZE / 2, "%s", spl_entry.name.c_str());
}

static void draw_projectile_damage(GameState* gs, const Projectile& projectile,
		const CoreStats& core, int x, int y) {
	const char* dmg_prefix = "Damage Bonus: ";
	ProjectileEntry& pentry = projectile.projectile_entry();
	if (pentry.is_unarmed()) {
		dmg_prefix = "Damage: ";
	}
	Range dmg = pentry.damage.calculate_range(core);
	if (dmg.max > 0) {
		draw_range_with_prefix(gs, x, y, dmg_prefix, COL_PALE_YELLOW, dmg,
				COL_PALE_RED);
	}
}

//Drawn only as part of other draw_console_<something>_description functions
static void draw_projectile_description_overlay(GameState* gs,
		const Projectile& projectile) {
	PlayerInst* p = gs->local_player();
	EffectiveStats& estats = p->effective_stats();

	ProjectileEntry& pentry = projectile.projectile_entry();
	GameTextConsole& console = gs->game_console();

	BBox bbox(console.bounding_box());

	int stat_x = bbox.center_x();
	int stat_sy = bbox.y1 + TILE_SIZE / 2 + 4;

	draw_projectile_damage(gs, projectile, estats.core, stat_x, stat_sy);
}

//Drawn only as part of other draw_console_<something>_description functions
static void draw_weapon_description_overlay(GameState* gs,
		const Projectile& projectile) {
	PlayerInst* p = gs->local_player();
	EffectiveStats& estats = p->effective_stats();

	ProjectileEntry& pentry = projectile.projectile_entry();
	GameTextConsole& console = gs->game_console();

	BBox bbox(console.bounding_box());

	int stat_x = bbox.center_x();
	int stat_sy = bbox.y1 + TILE_SIZE / 2 + 4;

	draw_projectile_damage(gs, projectile, estats.core, stat_x, stat_sy);
}

void draw_console_item_description(GameState* gs, const Item& item) {
	GameTextConsole& console = gs->game_console();

	if (console.has_content_already()) {
		return;
	}
	ItemEntry& ientry = item.item_entry();
	console.draw_box(gs);
	BBox bbox(console.bounding_box());
	draw_item_icon_and_name(gs, ientry, Colour(), bbox.x1 + 4, bbox.y1 + 4);
	gl_printf_y_centered(gs->primary_font(), COL_PALE_YELLOW,
			bbox.center_x() / 2, bbox.y1 + TILE_SIZE / 2 + 4, "%s",
			equip_type_description(ientry));

	const int MAX_WIDTH = bbox.width() - TILE_SIZE;

	gl_printf_bounded(gs->primary_font(), COL_LIGHT_GRAY,
			bbox.x1 + TILE_SIZE / 2, bbox.y1 + TILE_SIZE + 9, MAX_WIDTH, "%s",
			ientry.description.c_str());

	if (item.is_projectile()) {
		draw_projectile_description_overlay(gs, item.as_projectile());
	} else if (item.is_weapon()) {

	}
}

void draw_console_spell_description(GameState* gs, SpellEntry& spl_entry) {
	GameTextConsole& console = gs->game_console();

	if (console.has_content_already()) {
		return;
	}

	console.draw_box(gs);
	BBox bbox(console.bounding_box());
	draw_spell_icon_and_name(gs, spl_entry, Colour(), bbox.x1 + 4, bbox.y1 + 4);
	draw_stat_text(gs, bbox.center_x() / 2, bbox.y1 + TILE_SIZE / 2 + 4,
			"MP cost: ", COL_PALE_YELLOW, spl_entry.mp_cost, COL_PALE_RED);

	const int MAX_WIDTH = bbox.width() - TILE_SIZE;

	gl_printf_bounded(gs->primary_font(), COL_LIGHT_GRAY,
			bbox.x1 + TILE_SIZE / 2, bbox.y1 + TILE_SIZE + 9, MAX_WIDTH, "%s",
			spl_entry.description.c_str());

	if (spl_entry.projectile.valid_projectile()) {
		draw_projectile_description_overlay(gs, spl_entry.projectile);
	}
}
