/*
 * content_draw_util.cpp:
 *  Common routines for content descriptions and drawing
 */

#include "../data/armour_data.h"
#include "../data/item_data.h"
#include "../data/projectile_data.h"
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
	if (speed < 1.5) {
		return "Very Slow";
	} else if (speed < 3) {
		return "Slow";
	} else if (speed < 5) {
		return "Medium";
	} else if (speed < 8) {
		return "Fast";
	}
	return "Very Fast";
}

static const char* range_description(int range) {
	if (range < 15) {
		return "Melee";
	} else if (range < 64) {
		return "Close";
	} else if (range < 200) {
		return "Medium";
	}
	return "Long";
}

const char* monster_speed_description(int speed) {
	return NULL;
}

static void draw_stat_with_prefix(GameState* gs, const char* prefix, int stat,
		int x, int y, Colour prefix_col = COL_PALE_YELLOW, Colour stat_col =
				COL_PALE_RED) {
	Pos p = gl_printf_y_centered(gs->primary_font(), prefix_col, x, y, "%s",
			prefix);
	gl_printf_y_centered(gs->primary_font(), stat_col, p.x + x, y, "%d", stat);
}
static void draw_descript_with_prefix(GameState* gs, const char* prefix,
		const char* descript, int x, int y, Colour prefix_col = COL_PALE_YELLOW,
		Colour stat_col = COL_PALE_RED) {
	Pos p = gl_printf_y_centered(gs->primary_font(), prefix_col, x, y, "%s",
			prefix);
	gl_printf_y_centered(gs->primary_font(), stat_col, p.x + x, y, "%s",
			descript);
}

static void draw_range_with_prefix(GameState* gs, const char* prefix,
		const Range& r, int x, int y, Colour prefix_col = COL_PALE_YELLOW,
		Colour stat_col = COL_PALE_RED) {
	Pos p = gl_printf_y_centered(gs->primary_font(), prefix_col, x, y, "%s",
			prefix);
	if (r.min == r.max) {
		gl_printf_y_centered(gs->primary_font(), stat_col, p.x + x, y, "%d",
				r.min);
	} else {
		gl_printf_y_centered(gs->primary_font(), stat_col, p.x + x, y,
				"%d to %d", r.min, r.max);
	}
}

static void draw_statmult_with_prefix(GameState* gs, const char* prefix,
		const CoreStatMultiplier& statmult, const CoreStats& core, int x, int y,
		bool dontdrawif0 = false) {
	Range dmg = statmult.calculate_range(core);
	if (!dontdrawif0 || dmg.max > 0) {
		draw_range_with_prefix(gs, prefix, dmg, x, y, COL_PALE_YELLOW,
				COL_PALE_GREEN);
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

	bool is_unarmed = pentry.is_unarmed();

	int stat_x = bbox.x1 + TILE_SIZE / 2, stat_sy = bbox.y1 + TILE_SIZE * 1.5
			+ 4;
	int interval_x = bbox.width() / 4, interval_y = bbox.height() / 6;

	//First row
	draw_descript_with_prefix(gs, "Speed: ",
			projectile_speed_description(pentry.speed), stat_x + interval_x,
			stat_sy, COL_PALE_YELLOW, COL_PALE_GREEN);

	//Second row
	draw_statmult_with_prefix(gs, is_unarmed ? "Damage: " : "Damage Bonus: ",
			pentry.damage, estats.core, stat_x, stat_sy + interval_y);
	draw_statmult_with_prefix(gs, is_unarmed ? "Power: " : "Power Bonus: ",
			pentry.power, estats.core, stat_x + interval_x,
			stat_sy + interval_y);

	//Third row
	if (is_unarmed) {
		draw_stat_with_prefix(gs, "Cooldown: ", pentry.cooldown, stat_x,
				stat_sy + interval_y * 2);
		draw_descript_with_prefix(gs, "Range: ",
				range_description(pentry.range), stat_x + interval_x,
				stat_sy + interval_y * 2, COL_PALE_YELLOW, COL_PALE_GREEN);
	}
}

//Drawn only as part of other draw_console_<something>_description functions
static void draw_weapon_description_overlay(GameState* gs,
		const Weapon& weapon) {
	PlayerInst* p = gs->local_player();
	EffectiveStats& estats = p->effective_stats();

	WeaponEntry& wentry = weapon.weapon_entry();
	GameTextConsole& console = gs->game_console();

	BBox bbox(console.bounding_box());

	int stat_x = bbox.x1 + TILE_SIZE / 2, stat_sy = bbox.y1 + TILE_SIZE * 1.5
			+ 4;
	int interval_x = bbox.width() / 4, interval_y = bbox.height() / 6;

	//First row
//	draw_descript_with_prefix(gs, "Speed: ",
//			projectile_speed_description(pentry.speed), stat_x + interval_x,
//			stat_sy, COL_PALE_YELLOW, COL_PALE_GREEN);

//Second row
	draw_statmult_with_prefix(gs, "Damage: ", wentry.damage, estats.core,
			stat_x, stat_sy + interval_y);
	draw_statmult_with_prefix(gs, "Power: ", wentry.power, estats.core,
			stat_x + interval_x, stat_sy + interval_y);

	//Third row
	draw_stat_with_prefix(gs, "Cooldown: ", wentry.cooldown, stat_x,
			stat_sy + interval_y * 2);
	draw_descript_with_prefix(gs, "Range: ", range_description(wentry.range),
			stat_x + interval_x, stat_sy + interval_y * 2, COL_PALE_YELLOW,
			COL_PALE_GREEN);
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

static void draw_labelled_sprite(GameState* gs, sprite_id sprite,
		const char* text, Colour col, int x, int y) {
	gl_draw_sprite(sprite, x, y);
	gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE,
			COL_PALE_YELLOW.with_alpha(50));
	/* Draw spell name */
	gl_printf_y_centered(gs->primary_font(), col, x + TILE_SIZE * 1.25,
			y + TILE_SIZE / 2, "%s", text);
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
	gl_printf_y_centered(gs->primary_font(), COL_WHITE, bbox.x1 + TILE_SIZE / 2,
			bbox.y1 + TILE_SIZE * 1.5 + 4, "%s",
			equip_type_description(ientry));

	const int x_offset = bbox.width() / 6, y_offset = 4 + TILE_SIZE / 2;
	const int max_width = bbox.width() - x_offset * 2;

	gl_printf_y_centered_bounded(gs->primary_font(), COL_LIGHT_GRAY,
			bbox.x1 + x_offset, bbox.y1 + y_offset, max_width, "%s",
			ientry.description.c_str());

	if (item.is_projectile()) {
		draw_projectile_description_overlay(gs, item.as_projectile());
	} else if (item.is_weapon()) {
		draw_weapon_description_overlay(gs, item.as_weapon());
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
	draw_stat_with_prefix(gs, "MP cost: ", spl_entry.mp_cost,
			bbox.x1 + TILE_SIZE / 2, bbox.y1 + TILE_SIZE * 1.5 + 4);

	const int x_offset = bbox.width() / 6, y_offset = 4 + TILE_SIZE / 2;
	const int max_width = bbox.width() - x_offset * 2;

	gl_printf_y_centered_bounded(gs->primary_font(), COL_LIGHT_GRAY,
			bbox.x1 + x_offset, bbox.y1 + y_offset, max_width, "%s",
			spl_entry.description.c_str());

	if (spl_entry.projectile.valid_projectile()) {
		draw_projectile_description_overlay(gs, spl_entry.projectile);
	}
}

void draw_console_enemy_description(GameState* gs, EnemyEntry& eentry) {
	GameTextConsole& console = gs->game_console();

	if (console.has_content_already()) {
		return;
	}

	console.draw_box(gs);
	BBox bbox(console.bounding_box());
	draw_labelled_sprite(gs, eentry.enemy_sprite, eentry.name.c_str(), Colour(),
			bbox.x1 + 4, bbox.y1 + 4);

//	draw_stat_with_prefix(gs, "MP cost: ", spl_entry.mp_cost,
//			bbox.x1 + TILE_SIZE / 2, bbox.y1 + TILE_SIZE * 1.5 + 4);

	const int x_offset = bbox.width() / 6, y_offset = 4 + TILE_SIZE / 2;
	const int max_width = bbox.width() - x_offset * 2;

	gl_printf_y_centered_bounded(gs->primary_font(), COL_LIGHT_GRAY,
			bbox.x1 + x_offset, bbox.y1 + y_offset, max_width, "%s",
			eentry.description.c_str());

//	if (spl_entry.projectile.valid_projectile()) {
//		draw_projectile_description_overlay(gs, spl_entry.projectile);
//	}
}

