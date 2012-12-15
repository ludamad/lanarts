/*
 * main_menu.cpp:
 *  Implements start menu and other actions
 */

#include <iostream>
#include <vector>
#include <ctime>

#include <SDL.h>
#include <SDL_opengl.h>

#include <lua.hpp>

#include <lcommon/Timer.h>
#include <ldraw/Font.h>
#include <ldraw/DrawOptions.h>

#include "../data/game_data.h"

#include "../draw/colour_constants.h"
#include "../display/display.h"

#include "gamestate/GameState.h"

#include "../interface/ButtonInst.h"
#include "../interface/TextBoxInst.h"

#include "../interface/components_draw.h"
#include "../interface/DrawCallbackInst.h"
#include "../lua/lua_api.h"

#include "objects/enemy/EnemyInst.h"
#include "objects/player/PlayerInst.h"
#include "objects/AnimatedInst.h"

#include "menus.h"

// **************  Class selection button functions ************** ///
static void choose_class(GameState* gs, GameInst* _, void* name) {
	class_id classn = get_class_by_name((const char*)name);
	class_id& settings_class = gs->game_settings().classtype;
	if (settings_class == classn) {
		settings_class = -1;
	} else {
		settings_class = classn;
	}
}
static void draw_class_option(GameState* gs, GameInst* inst, void* name) {
	ButtonInst* button = dynamic_cast<ButtonInst*>(inst);
	if (gs->game_settings().classtype == get_class_by_name((const char*)name)) {
		button->draw_colour() = COL_GOLD;
		button->hover_colour() = COL_GOLD;
	} else {
		button->draw_colour() = COL_WHITE;
		button->hover_colour() = COL_MUTED_YELLOW;
	}
}
static void setup_class_selection_button(GameState* gs, int x, int y,
		const char* icon, const char* name) {
	gs->add_instance(
			new ButtonInst(name, get_sprite_by_name(icon), x, y,
					ObjCallback(choose_class, (void*)name),
					ObjCallback(draw_class_option, (void*)name), COL_WHITE,
					COL_GOLD));
}
static void setup_class_selection_buttons(GameState* gs, int x, int y) {
	gs->game_settings().classtype = -1; // Make sure class is not set
	setup_class_selection_button(gs, x, y, "wizard_icon", "Mage");
	x += 128;
	setup_class_selection_button(gs, x, y, "fighter_icon", "Fighter");
	x += 128;
	setup_class_selection_button(gs, x, y, "archer_icon", "Archer");
	x += 128;
//	setup_class_selection_button(gs, x, y, "druid_icon", "Druid");

}

// **************  Class selection button functions ************** ///

// **************  Menu leaving button functions ************** ///

const int SETTINGS_BOX_WIDTH = 180;
const int CONFIG_MENU_AREA_WIDTH = 640;
const int CONFIG_MENU_AREA_HEIGHT = 480;

static void draw_choose_class_to_begin(GameState* gs, GameInst* inst, void* _) {
	// We cheat here and bundle step and draw actions
	// The interface will always be drawn once a step
	GameSettings& settings = gs->game_settings();
	if (settings.classtype == -1) {

		using namespace ldraw;
		const Font& font = gs->font();
		font.draw(DrawOptions(CENTER, COL_PALE_RED), inst->pos(),
				"Choose your Class!");
	}
}

static void onclick_goback(GameState* gs, GameInst* inst, void* exitcode) {
	*(int*)exitcode = -1;
}
static void onclick_start(GameState* gs, GameInst* inst, void* exitcode) {
	GameSettings& settings = gs->game_settings();
	if (settings.classtype != -1) {
		*(int*)exitcode = +1;
	}
}

static void setup_config_exit_options(GameState* gs, int* exitcode, int x,
		int y) {
	int sbx = x - CONFIG_MENU_AREA_WIDTH / 2 + 128;
	int ebx = x + CONFIG_MENU_AREA_WIDTH / 2 - 128;
	gs->add_instance(
			new DrawCallbackInst(Pos(x, y), ObjCallback(),
					draw_choose_class_to_begin));
	gs->add_instance(
			button_inst("Back", x - TILE_SIZE * 2, y + TILE_SIZE,
					ObjCallback(onclick_goback, exitcode)));
	gs->add_instance(
			button_inst("Start", x + TILE_SIZE * 2, y + TILE_SIZE,
					ObjCallback(onclick_start, exitcode)));
}

// **************  Menu leaving button functions ************** ///

static void draw_respawn_toggle(GameState* gs, GameInst* inst, void* _) {
	// We cheat here and bundle step and draw actions
	// The interface will always be drawn once a step
	const int width = SETTINGS_BOX_WIDTH;
	const int height = TILE_SIZE + 2;
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
		Colour bbox_col = hover ? COL_GOLD : COL_PALE_RED;
		draw_setting_box(gs, bbox, bbox_col, hardcore, COL_PALE_RED,
				"Hardcore (No respawn!)", COL_PALE_RED);
	}
	if (clicked) {
		settings.regen_on_death = !settings.regen_on_death;
	}
}

static void draw_connection_toggle(GameState* gs, GameInst* inst, void* _) {
	// We cheat here and bundle step and draw actions
	// The interface will always be drawn once a step
	const int width = SETTINGS_BOX_WIDTH;
	const int height = TILE_SIZE + 2;
	GameSettings& settings = gs->game_settings();
	BBox bbox(inst->x, inst->y, inst->x + width, inst->y + height);
	bool hover = bbox.contains(gs->mouse_pos());
	bool clicked = hover && gs->mouse_left_click();

	sprite_id client = get_sprite_by_name("client setting icon");
	sprite_id server = get_sprite_by_name("server setting icon");
	sprite_id single_player = get_sprite_by_name("single player setting icon");

	if (settings.conntype == GameSettings::CLIENT) {
		Colour bbox_col = hover ? COL_GOLD : COL_MUTED_GREEN;

		draw_setting_box(gs, bbox, bbox_col, client, COL_MUTED_GREEN,
				"Connect to a Game", COL_MUTED_GREEN);
	} else if (settings.conntype == GameSettings::SERVER) {
		Colour bbox_col = hover ? COL_GOLD : COL_PALE_RED;
		draw_setting_box(gs, bbox, bbox_col, server, COL_PALE_RED,
				"Host a game", COL_PALE_RED);
	} else {
		Colour bbox_col = hover ? COL_GOLD : COL_BABY_BLUE;
		draw_setting_box(gs, bbox, bbox_col, single_player, COL_BABY_BLUE,
				"Single-player", COL_BABY_BLUE);
	}

	if (hover && clicked) {
		settings.conntype = GameSettings::connection_type(
				(settings.conntype + 1) % 3);
	}
}

static void draw_speed_toggle(GameState* gs, GameInst* inst, void* _) {
	// We cheat here and bundle step and draw actions
	// The interface will always be drawn once a step
	const int width = SETTINGS_BOX_WIDTH;
	const int height = TILE_SIZE + 2;
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

static void __setup_next_field_location(bool& on_first_row, int& x, int& y) {
	on_first_row = !on_first_row;
	x += CONFIG_MENU_AREA_WIDTH / 2; // SETTINGS_BOX_WIDTH / 2;
	if (on_first_row) {
		x -= CONFIG_MENU_AREA_WIDTH;
		y += TILE_SIZE * 2;
	}
}

static void text_update_string(GameState* gs, GameInst* inst, void* strp) {
	TextBoxInst* textbox = dynamic_cast<TextBoxInst*>(inst);
	std::string * strptr = (std::string*)strp;
	LANARTS_ASSERT(textbox);
	*strptr = textbox->text();
}

static bool is_valid_ip(const std::string& ip) {
	int dots = 0;
	const char* section_start = &ip[0];
	int section_has_digit = false;
	for (int i = 0; i < ip.size(); i++) {
		if (ip[i] == '.') {
			if (!section_has_digit)
				return false;
//			int amnt = atoi(section_start);
//			if (amnt < 0 || amnt > 255) {
//				return false;
//			}
			section_start = &ip[i + 1];
			dots++;
		} else if (isdigit(ip[i])) {
			section_has_digit = true;
		} else {
			return false;
		}
	}

	return true;
}

static void text_update_ip(GameState* gs, GameInst* inst, void* strp) {
	TextBoxInst* textbox = dynamic_cast<TextBoxInst*>(inst);
	LANARTS_ASSERT(textbox);
	std::string * strptr = (std::string*)strp;
// Check if valid IP or hostname like localhost
	bool valid = is_valid_ip(textbox->text().c_str())
			|| textbox->text() == "localhost";
	textbox->mark_validity(valid);
	if (valid) {
		*strptr = textbox->text();
	}
}

static void text_update_int(GameState* gs, GameInst* inst, void* intp) {
	TextBoxInst* textbox = dynamic_cast<TextBoxInst*>(inst);
	const std::string& text = textbox->text();
	int* intptr = (int*)intp;
	LANARTS_ASSERT(textbox);
// Check if valid string of all digits:
	bool valid = !text.empty();
	for (int i = 0; i < text.size(); i++) {
		if (!isdigit(text[i])) {
			valid = false;
		}
	}
	textbox->mark_validity(valid);
// Only convert if valid
	if (valid) {
		*intptr = atoi(text.c_str());
	}
}
static void setup_username_field(GameState* gs, int x, int y) {
	GameSettings& settings = gs->game_settings();
	gs->add_instance(
			animated_inst(Pos(x, y - 20), "Enter your name:", COL_YELLOW));
	ObjCallback callback(text_update_string, &settings.username);
	TextBoxInst* textbox = new TextBoxInst(
			BBox(x, y, x + SETTINGS_BOX_WIDTH, y + TILE_SIZE), 20,
			settings.username, callback);
	gs->add_instance(textbox);

}
static void text_sync_int(GameState* gs, GameInst* inst, void* intp) {
	char buff[32];
	TextBoxInst* textbox = dynamic_cast<TextBoxInst*>(inst);
	int* intptr = (int*)intp;
	snprintf(buff, 32, "%d", *intptr);
	textbox->set_text(buff);
	textbox->mark_validity(true);
}
static void text_sync_string(GameState* gs, GameInst* inst, void* intp) {
	char buff[32];
	TextBoxInst* textbox = dynamic_cast<TextBoxInst*>(inst);
	textbox->set_text(gs->game_settings().ip);
	textbox->mark_validity(true);
}

static void setup_port_field(GameState* gs, int x, int y) {
	GameSettings& settings = gs->game_settings();
	gs->add_instance(
			animated_inst(Pos(x, y - 20), "Connection port:", COL_YELLOW));
	ObjCallback typing_callback(text_update_int, &settings.port);
	ObjCallback deselect_callback(text_sync_int, &settings.port);
	TextBoxInst* textbox = new TextBoxInst(
			BBox(x, y, x + SETTINGS_BOX_WIDTH, y + TILE_SIZE), 20, "",
			typing_callback, deselect_callback);
	text_sync_int(gs, textbox, &settings.port);
	gs->add_instance(textbox);
}

static void setup_ip_field(GameState* gs, int x, int y) {
	GameSettings& settings = gs->game_settings();
	gs->add_instance(animated_inst(Pos(x, y - 20), "Host IP:", COL_YELLOW));
	ObjCallback typing_callback(text_update_ip, &settings.ip);
	ObjCallback deselect_callback(text_sync_string, &settings.ip);
	TextBoxInst* textbox = new TextBoxInst(
			BBox(x, y, x + SETTINGS_BOX_WIDTH, y + TILE_SIZE), 20, "",
			typing_callback, deselect_callback);
	text_sync_string(gs, textbox, &settings.ip);
	gs->add_instance(textbox);
}

static void setup_config_options(GameState* gs, int* exitcode, int x, int y) {
	int start_x = x;
	int start_y = y;
	setup_class_selection_buttons(gs, start_x - 128, y);

	y += 128;
	int opt_start_x = x - CONFIG_MENU_AREA_WIDTH / 4 - SETTINGS_BOX_WIDTH / 2;
	int opt_x = opt_start_x;
	bool on_first_row = true;

	gs->add_instance(
			new DrawCallbackInst(Pos(opt_x, y), ObjCallback(),
					draw_connection_toggle));
	__setup_next_field_location(on_first_row, opt_x, y);

	if (gs->game_settings().conntype == GameSettings::NONE) {
		gs->add_instance(
				new DrawCallbackInst(Pos(opt_x, y), ObjCallback(),
						draw_respawn_toggle));
		__setup_next_field_location(on_first_row, opt_x, y);
	}
	gs->add_instance(
			new DrawCallbackInst(Pos(opt_x, y), ObjCallback(),
					draw_speed_toggle));
	__setup_next_field_location(on_first_row, opt_x, y);

	setup_username_field(gs, opt_x, y);
	__setup_next_field_location(on_first_row, opt_x, y);
	if (gs->game_settings().conntype != GameSettings::NONE) {
		setup_port_field(gs, opt_x, y);
		__setup_next_field_location(on_first_row, opt_x, y);
	}
	if (gs->game_settings().conntype == GameSettings::CLIENT) {
		setup_ip_field(gs, opt_x, y);
	}
	setup_config_exit_options(gs, exitcode, start_x,
			start_y + CONFIG_MENU_AREA_HEIGHT - 192);
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

int class_menu(GameState* gs, int width, int height) {
	setup_menu_again: int exitcode = 0;
	int halfw = width / 2;

	GameView prevview = gs->view();
	GameLevelState* oldlevel = gs->get_level();

	gs->set_level(new GameLevelState(-1, width, height));
	gs->view().x = 0;
	gs->view().y = 0;
	int classx = halfw;
	int classy = (height / 2 - CONFIG_MENU_AREA_HEIGHT / 2) * 1.5 + 64;

	logo_add(gs, classx, classy);
	setup_config_options(gs, &exitcode, classx, classy);
	GameSettings::connection_type conntype = gs->game_settings().conntype;

	for (; exitcode == 0;) {
		if (!gs->update_iostate()) {
			return +1;
		}
		if (conntype != gs->game_settings().conntype) {
			delete gs->get_level();
			gs->set_level(oldlevel);
			goto setup_menu_again;
		}
		gs->get_level()->game_inst_set().step(gs);
		gs->draw(false);

	}

	delete gs->get_level();

	gs->set_level(oldlevel);
	gs->view() = prevview;
	if (exitcode > 0) {
		return 0;
	}
	return exitcode;
}

