/*
 * content_draw_util.cpp:
 *  Common routines for content descriptions and drawing
 */

#include "../display/colour_constants.h"
#include "../display/display.h"
#include "../display/sprite_data.h"
#include "../gamestate/GameState.h"
#include "../objects/player/PlayerInst.h"

#include "../stats/items/EquipmentEntry.h"
#include "../stats/items/ProjectileEntry.h"
#include "../stats/items/WeaponEntry.h"

#include "../stats/items/ItemEntry.h"
#include "../stats/items/items.h"

#include "../stats/SpellEntry.h"

#include "console_description_draw.h"

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

const int WIDTH_THRESHOLD = 600; // Represents an 800 width resolution typically
const int MAX_WIDTH = 800; // Represents an 1000 width resolution typically
class DescriptionBoxHelper {
public:
	DescriptionBoxHelper(const BBox& bbox) :
			bbox(bbox), draw_index(0) {
		cols_per_row = 4;
		if (this->bbox.width() > MAX_WIDTH) {
			this->bbox.x2 = this->bbox.x1 + MAX_WIDTH;
		}
	}
	void draw_prefix(GameState* gs, const Colour& col, const char* fmt, ...) {
		Pos pos = get_next_draw_position();
		char buff[512];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buff, 512, fmt, ap);
		va_end(ap);
		Dim value_offset = gl_printf_y_centered(gs->primary_font(), col, pos.x,
				pos.y, "%s", buff);
		value_draw_pos = Pos(pos.x + value_offset.w, pos.y);
	}
	void draw_value(GameState* gs, const Colour& col, const char* fmt, ...) {
		Pos pos = value_draw_pos;
		char buff[512];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buff, 512, fmt, ap);
		va_end(ap);
		gl_printf_y_centered(gs->primary_font(), col, pos.x, pos.y, "%s", buff);
	}
	int width() const {
		return bbox.width();
	}
	int height() const {
		return bbox.height();
	}
private:
	static const int SX = TILE_SIZE / 2, SY = TILE_SIZE * 1.5 + 4;

	Pos get_next_draw_position() {
		int xinterval = bbox.width() / cols_per_row;
		int yinterval = bbox.height() / 6;

		int col = draw_index % cols_per_row;
		int row = draw_index / cols_per_row;
		draw_index++;

		return Pos(SX + xinterval * col, SY + yinterval * row);
	}
	int cols_per_row;
	/* Members */
	int draw_index;
	BBox bbox;
	Pos value_draw_pos;
};

static void draw_base_entry_overlay(GameState* gs, BaseDataEntry& entry) {
	GameTextConsole& console = gs->game_console();
	BBox bbox(console.bounding_box());
	int descriptxoff = TILE_SIZE * 1.25;
	Dim itemdim = draw_icon_and_name(gs, entry, Colour(), bbox.x1 + 4,
			bbox.y1 + 4, TILE_SIZE * 1.25 - 4, TILE_SIZE / 4);
	Dim typedim = gl_printf_y_centered(gs->primary_font(), COL_LIGHT_GRAY,
			bbox.x1 + descriptxoff, bbox.y1 + TILE_SIZE * .75 + 4, "%s",
			entry.entry_type());

	const int x_offset = bbox.width() / 6, y_offset = 4 + TILE_SIZE / 2;
	const int max_width = bbox.width() - x_offset * 2;

	int descript_xoffset = x_offset;
	if (descript_xoffset < WIDTH_THRESHOLD / 6) {
		descript_xoffset = WIDTH_THRESHOLD / 6;
	}
	if (descript_xoffset < itemdim.w + descriptxoff + 4) {
		descript_xoffset = itemdim.w + descriptxoff + 4;
	}
	if (descript_xoffset < typedim.w + descriptxoff + 4) {
		descript_xoffset = typedim.w + descriptxoff + 4;
	}

	gl_printf_y_centered_bounded(gs->primary_font(), COL_LIGHT_GRAY,
			bbox.x1 + descript_xoffset, bbox.y1 + y_offset, max_width, "%s",
			entry.description.c_str());
}

Dim draw_icon_and_name(GameState* gs, BaseDataEntry& entry, Colour col, int x,
		int y, int xoffset, int yoffset) {
	gl_draw_sprite(entry.get_sprite(), x, y);
	gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE,
			COL_PALE_YELLOW.with_alpha(50));
	/* Draw item name */
	return gl_printf_y_centered(gs->primary_font(), col, x + xoffset,
			y + yoffset, "%s", entry.name.c_str());
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

//Drawn only as part of other draw_console_<something>_description functions
static void draw_value(GameState* gs, DescriptionBoxHelper& dbh,
		const char* name, int bonus, const Colour& prefixcol = COL_GREEN,
		const Colour& valuecol = COL_PALE_GREEN, bool optional = true) {
	if (!optional || bonus > 0) {
		dbh.draw_prefix(gs, prefixcol, "%s", name);
		dbh.draw_value(gs, valuecol, "%d", bonus);
	}
}
static void draw_statmult(GameState* gs, DescriptionBoxHelper& dbh,
		const char* name, CoreStatMultiplier& mult, CoreStats& core,
		const Colour& prefixcol = COL_GREEN, const Colour& valuecol =
				COL_PALE_GREEN, bool optional = true) {
	if (!optional || !mult.is_empty()) {
		dbh.draw_prefix(gs, prefixcol, "%s", name);
		Range value_range = mult.calculate_range(core);
		if (value_range.min == value_range.max) {
			dbh.draw_value(gs, valuecol, "%d", value_range.min);
		} else {
			dbh.draw_value(gs, valuecol, "%d to %d", value_range.min,
					value_range.max);
		}
	}
}

static void draw_stat_bonuses_overlay(GameState* gs, DescriptionBoxHelper& dbh,
		CoreStats& core) {
	draw_value(gs, dbh, "+HP: ", core.max_hp);
	draw_value(gs, dbh, "+MP: ", core.max_mp);
	draw_value(gs, dbh, "+Strength: ", core.strength);
	draw_value(gs, dbh, "+Defence: ", core.defence);
	draw_value(gs, dbh, "+Magic: ", core.magic);
	draw_value(gs, dbh, "+Will: ", core.willpower);
}
static void draw_defence_bonuses_overlay(GameState* gs,
		DescriptionBoxHelper& dbh, ArmourStats& armour) {
	PlayerInst* p = gs->local_player();
	CoreStats& core = p->effective_stats().core;
	const char* dmgreduct = "+Physical Reduction: ";
	const char* dmgresist = "+Physical Resistance: ";
	const char* mdmgreduct = "+Magic Reduction: ";
	const char* mdmgresist = "+Magic Resistance: ";
	if (dbh.width() < WIDTH_THRESHOLD) {
		dmgreduct = "+Reduction: ";
		dmgresist = "+Resist: ";
		mdmgreduct = "+MReduction: ";
		mdmgresist = "+MResist: ";
	}
	draw_statmult(gs, dbh, dmgreduct, armour.damage_reduction, core, COL_GOLD);
	draw_statmult(gs, dbh, dmgresist, armour.resistance, core, COL_GOLD);
	draw_statmult(gs, dbh, mdmgreduct, armour.magic_reduction, core,
			COL_BABY_BLUE);
	draw_statmult(gs, dbh, mdmgresist, armour.magic_resistance, core,
			COL_BABY_BLUE);
}
static void draw_damage_bonuses_overlay(GameState* gs,
		DescriptionBoxHelper& dbh, DamageStats& damage) {
	PlayerInst* p = gs->local_player();
	CoreStats& core = p->effective_stats().core;
	draw_statmult(gs, dbh, "+Damage: ", damage.damage_stats, core);
	draw_statmult(gs, dbh, "+Power: ", damage.power_stats, core);
}
static void draw_equipment_description_overlay(GameState* gs,
		DescriptionBoxHelper& dbh, const Item& item) {
	EquipmentEntry& entry = item.equipment_entry();
	draw_stat_bonuses_overlay(gs, dbh, entry.core_stat_modifier());
	draw_defence_bonuses_overlay(gs, dbh, entry.armour_modifier());
	draw_damage_bonuses_overlay(gs, dbh, entry.damage_modifier());
}

static void draw_attack_description_overlay(GameState* gs,
		DescriptionBoxHelper& dbh, Attack& attack) {
	PlayerInst* p = gs->local_player();
	CoreStats& core = p->effective_stats().core;
	draw_statmult(gs, dbh, "Damage: ", attack.damage_stats(), core,
			COL_PALE_YELLOW, COL_PALE_GREEN, false);
	draw_statmult(gs, dbh, "Power: ", attack.power_stats(), core,
			COL_PALE_YELLOW, COL_PALE_GREEN, false);

	// Damage penetration

	dbh.draw_prefix(gs, COL_PALE_YELLOW, "Piercing: ");
	int piercing = round(100 - attack.resistability() * 100);
	Colour statcol = piercing < 0 ? COL_PALE_RED : COL_PALE_GREEN;
	dbh.draw_value(gs, statcol, "%d%%", piercing);

	if (attack.range >= 15) {
		dbh.draw_prefix(gs, COL_PALE_YELLOW, "Range: ");
		dbh.draw_value(gs, COL_PALE_GREEN, "%s",
				range_description(attack.range));
	}
	draw_value(gs, dbh, "Cooldown: ", attack.cooldown, COL_PALE_YELLOW,
			COL_PALE_RED);
}

static void draw_weapon_description_overlay(GameState* gs,
		DescriptionBoxHelper& dbh, const Weapon& weapon) {
	WeaponEntry& entry = weapon.weapon_entry();
	PlayerInst* p = gs->local_player();
	EffectiveStats& estats = p->effective_stats();
	draw_attack_description_overlay(gs, dbh, entry.attack);
	draw_equipment_description_overlay(gs, dbh, weapon);
}

static void draw_projectile_description_overlay(GameState* gs,
		DescriptionBoxHelper& dbh, const Projectile& projectile) {
	ProjectileEntry& entry = projectile.projectile_entry();
	PlayerInst* p = gs->local_player();
	EffectiveStats& estats = p->effective_stats();
	if (entry.is_standalone()) {
		draw_attack_description_overlay(gs, dbh, entry.attack);
	}
	draw_equipment_description_overlay(gs, dbh, projectile);
}

void draw_console_spell_description(GameState* gs, SpellEntry& entry) {
	GameTextConsole& console = gs->game_console();

	if (console.has_content_already()) {
		return;
	}
	console.draw_box(gs);
	draw_base_entry_overlay(gs, entry);

	DescriptionBoxHelper dbh(console.bounding_box());
	draw_value(gs, dbh, "MP cost: ", entry.mp_cost, COL_PALE_YELLOW,
			COL_PALE_RED);

	if (!entry.projectile.empty()) {
		draw_projectile_description_overlay(gs, dbh, entry.projectile);
	}
	draw_value(gs, dbh, "Cooldown: ", entry.cooldown, COL_PALE_YELLOW,
			COL_PALE_RED);
}

void draw_console_enemy_description(GameState* gs, EnemyEntry& entry) {
	GameTextConsole& console = gs->game_console();

	if (console.has_content_already()) {
		return;
	}

	console.draw_box(gs);
	draw_base_entry_overlay(gs, entry);
	CoreStats& ecore = entry.basestats.core;
	DescriptionBoxHelper dbh(console.bounding_box());
	draw_value(gs, dbh, "HP: ", ecore.hp, COL_PALE_YELLOW, COL_PALE_RED);
	draw_value(gs, dbh, "Strength: ", ecore.strength, COL_PALE_YELLOW,
			COL_PALE_RED);
	draw_value(gs, dbh, "Magic: ", ecore.magic, COL_PALE_YELLOW, COL_PALE_RED);
	draw_value(gs, dbh, "Defence: ", ecore.defence, COL_PALE_YELLOW,
			COL_PALE_RED);
	draw_value(gs, dbh, "Will: ", ecore.willpower, COL_PALE_YELLOW,
			COL_PALE_RED);
}

void draw_console_item_description(GameState* gs, const Item& item,
		ItemEntry& entry) {
	GameTextConsole& console = gs->game_console();

	if (console.has_content_already()) {
		return;
	}
	console.draw_box(gs);
	draw_base_entry_overlay(gs, entry);
	DescriptionBoxHelper dbh(console.bounding_box());

	if (is_item_projectile(entry)) {
		draw_projectile_description_overlay(gs, dbh, item);
	} else if (is_item_weapon(entry)) {
		draw_weapon_description_overlay(gs, dbh, item);
	} else if (item.is_equipment()) {
		draw_equipment_description_overlay(gs, dbh, item);
	}
}
