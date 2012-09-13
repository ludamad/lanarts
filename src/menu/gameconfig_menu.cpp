/*
 * main_menu.cpp:
 *  Implements start menu and other actions
 */

#include <iostream>
#include <vector>
#include <ctime>

#include <SDL.h>
#include <SDL_opengl.h>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "../data/game_data.h"

#include "../display/colour_constants.h"
#include "../display/display.h"

#include "../gamestate/GameState.h"

#include "../interface/ButtonInst.h"
#include "../interface/TextBoxInst.h"

#include "../interface/components_draw.h"
#include "../interface/DrawCallbackInst.h"
#include "../lua/lua_api.h"

#include "../objects/enemy/EnemyInst.h"
#include "../objects/player/PlayerInst.h"
#include "../objects/AnimatedInst.h"

#include "../util/Timer.h"

#include "menus.h"

static void choose_fighter(GameState* gs, GameInst* _, void* flag) {
	*(bool*)flag = true;
	gs->game_settings().classtype = get_class_by_name("Fighter");
}
static void choose_mage(GameState* gs, GameInst* _, void* flag) {
	*(bool*)flag = true;
	gs->game_settings().classtype = get_class_by_name("Mage");
}
static void choose_druid(GameState* gs, GameInst* _, void* flag) {
//	*(bool*) flag = true;
//	gs->game_settings().classtype = get_class_by_name("Druid");
}
static void choose_archer(GameState* gs, GameInst* _, void* flag) {
	*(bool*)flag = true;
	gs->game_settings().classtype = get_class_by_name("Archer");
}

const int SETTINGS_BOX_WIDTH = 180;
const int CONFIG_MENU_AREA_WIDTH = 640;
const int CONFIG_MENU_AREA_HEIGHT = 480;

static void draw_respawn_toggle(GameState* gs, GameInst* inst, void* _) {
	// We cheat here and bundle step and draw actions
	// The interface will always be drawn once a step
	const int width = SETTINGS_BOX_WIDTH;
	const int height = TILE_SIZE + 8;
	GameSettings& settings = gs->game_settings();
	BBox bbox(inst->x, inst->y, inst->x + width, inst->y + height);
	sprite_id hardcore = get_sprite_by_name("hardcore setting icon");
	sprite_id respawn = get_sprite_by_name("respawn setting icon");
	bool hover = bbox.contains(gs->mouse_pos());
	bool clicked = hover && gs->mouse_left_click();
	if (settings.regen_on_death) {
		Colour bbox_col = hover ? COL_GOLD : COL_WHITE;
		draw_setting_box(gs, bbox, bbox_col, respawn, COL_WHITE,
				"Respawn on Death", COL_WHITE);
	} else {
		Colour bbox_col = hover ? COL_GOLD : COL_RED;
		draw_setting_box(gs, bbox, bbox_col, hardcore, COL_PALE_RED,
				"Hardcore (No respawn!)", COL_PALE_RED);
	}
	if (clicked) {
		settings.regen_on_death = !settings.regen_on_death;
	}
}

static void draw_speed_toggle(GameState* gs, GameInst* inst, void* _) {
	// We cheat here and bundle step and draw actions
	// The interface will always be drawn once a step
	const int width = SETTINGS_BOX_WIDTH;
	const int height = TILE_SIZE + 8;
	GameSettings& settings = gs->game_settings();
	BBox bbox(inst->x, inst->y, inst->x + width, inst->y + height);
	draw_speed_box(gs, bbox);
	bool hover = bbox.contains(gs->mouse_pos());
	bool clicked = hover && gs->mouse_left_click();

	if (hover && clicked) {
		settings.time_per_step -= 2;
		if (settings.time_per_step < 10) {
			settings.time_per_step = 16;
		}
	}
}

static void setup_classmenu_buttons(GameState* gs, bool* exit, int x, int y) {
	ObjCallback chfighter(choose_fighter, exit);
	ObjCallback chmage(choose_mage, exit);
	ObjCallback chdruid(choose_druid, exit);
	ObjCallback charcher(choose_archer, exit);

	int icons_x = x - 192;
	y += 128;

	gs->add_instance(
			new ButtonInst("Mage", get_sprite_by_name("wizard_icon"), icons_x,
					y, chmage, COL_GOLD));
	icons_x += 128;
	gs->add_instance(
			new ButtonInst("Fighter", get_sprite_by_name("fighter_icon"),
					icons_x, y, chfighter, COL_GOLD));
	icons_x += 128;
	gs->add_instance(
			new ButtonInst("Archer", get_sprite_by_name("archer_icon"), icons_x,
					y, charcher, COL_GOLD));
	icons_x += 128;
	gs->add_instance(
			new ButtonInst("Druid", get_sprite_by_name("druid_icon"), icons_x,
					y, chdruid, COL_LIGHT_GRAY));
	icons_x += 128;

	y += 128 + TILE_SIZE;
	int opt_x = x - CONFIG_MENU_AREA_WIDTH / 4 - SETTINGS_BOX_WIDTH / 2;
	if (gs->game_settings().conntype == GameSettings::NONE) {
		gs->add_instance(
				new DrawCallbackInst(Pos(opt_x, y), ObjCallback(),
						draw_respawn_toggle));
		opt_x += CONFIG_MENU_AREA_WIDTH / 2; // SETTINGS_BOX_WIDTH / 2;
	}
	gs->add_instance(
			new DrawCallbackInst(Pos(opt_x, y), ObjCallback(),
					draw_speed_toggle));
}

static void logo_add(GameState* gs, int classx, int classy) {
	sprite_id logosprite = get_sprite_by_name("logo");
	SpriteEntry& entry = game_sprite_data.at(logosprite);
	int logoh = entry.height();
	int logoy = classy - logoh / 2;
	if (100 < logoy) {
		gs->add_instance(new AnimatedInst(Pos(classx, 100), logosprite));
	}
}

void class_menu(GameState* gs, int width, int height) {
	bool exit = false;
	int halfw = width / 2;

	GameView prevview = gs->view();
	GameLevelState* oldlevel = gs->get_level();

	gs->set_level(new GameLevelState(-1, width, height));
	gs->view().x = 0;
	gs->view().y = 0;
	int classx = halfw;
	int classy = (height / 2 - CONFIG_MENU_AREA_HEIGHT / 2) * 1.75;

	logo_add(gs, classx, classy);
	setup_classmenu_buttons(gs, &exit, classx, classy);

	for (; gs->update_iostate() && !exit;) {
		gs->get_level()->game_inst_set().step(gs);
		gs->draw(false);
	}

	delete gs->get_level();

	gs->set_level(oldlevel);
	gs->view() = prevview;
}

