/*
 * shapes_example.cpp:
 *  Exercises the shape drawing methods
 */

#include <SDL.h>
#include <GL/glu.h>

#include <SLB/Script.hpp>
#include <SLB/LuaCall.hpp>
#include <SLB/Table.hpp>

#include <common/lua/slb_mutabletable.h>
#include <common/lua/lua_lcommon.h>

#include <lua/lualib.h>

#include "../lua/lua_ldraw.h"

#include "../Colour.h"

#include "../colour_constants.h"

#include "../display.h"
#include "../draw.h"
#include "../Image.h"
#include "../Font.h"

static bool handle_event(SDL_Event* event) {
	SDLKey keycode = event->key.keysym.sym;
	SDLMod keymod = event->key.keysym.mod;

	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN: {
		break;
	}
	case SDL_QUIT: {
		return false;
	}
	case SDL_KEYDOWN: {
		if (keycode == SDLK_RETURN || keycode == SDLK_ESCAPE) {
			return false;
		}
		if (keycode == SDLK_F1) {
			ldraw::display_set_fullscreen(!ldraw::display_is_fullscreen());
		}
	}
		break;
	}
	return true;
}

typedef void (*DrawFunc)();

static void draw_loop(DrawFunc draw_func) {
	while (1) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (!handle_event(&event)) {
				return; // Exit draw loop
			}
		}
		ldraw::draw_start();
		draw_func();
		ldraw::draw_finish();
	}
}

static void draw_shapes() {
	ldraw::draw_rectangle(COL_LIGHT_RED, BBoxF(100, 100, 300, 300));
	ldraw::draw_circle(COL_LIGHT_BLUE, Posf(200, 200), 100);
}

ldraw::Image image;

static void draw_images() {
	BBox box(0, 0, 10, 10);
	FOR_EACH_BBOX(box, x, y) {
		image.draw(Pos(x * 40, y * 40));
	}
}

ldraw::Font font;

static void draw_text() {
	font.draw(COL_WHITE, Pos(0, 0), "Hello World!");
	font.draw_wrapped(ldraw::DrawOptions(ldraw::CENTER, COL_PALE_RED),
			Pos(200, 200), 250,
			"This text is wrapped because it's sort of long.");
}

lua_State* L;
SLB::Manager m;
SLB::MutableTable* globals;

static void draw_script() {
	SLB::LuaCall<void()> drawfunc(L, "draw");
	drawfunc();
}

static void setup_lua_state() {
	L = lua_open();
	m.registerSLB(L);

	globals = new SLB::MutableTable;
	globals->push(L);
	lua_replace(L, LUA_GLOBALSINDEX);

	luaL_openlibs(L);
	lua_register_lcommon(L, globals);

	lua_register_ldraw(L, globals);
}

int main(int argc, const char** argv) {
	ldraw::display_initialize(__FILE__, Dim(400, 400), false);
	image.initialize("sample.png");
	font.initialize("sample.ttf", 20);

	draw_loop(draw_shapes);
	draw_loop(draw_images);
	draw_loop(draw_text);

	setup_lua_state();

	luaL_dofile(L, "scripts/draw_shapes.lua");
	draw_loop(draw_script);

	luaL_dofile(L, "scripts/draw_images.lua");
	draw_loop(draw_script);

	luaL_dofile(L, "scripts/draw_text.lua");
	draw_loop(draw_script);

	luaL_dofile(L, "scripts/draw_animated.lua");
	printf(lua_tostring(L,-1));
	draw_loop(draw_script);

	lua_close(L);
}
