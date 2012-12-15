/*
 * lanarts_examples.cpp:
 *  Exercises some of the hard-to-unit test code
 */

#include <SDL.h>
#include <GL/glu.h>

#include <lua.hpp>

#include <yaml-cpp/yaml.h>

#include <common/mathutil.h>
#include <common/Timer.h>

#include <luawrap/LuaValue.h>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/calls.h>

#include <common/lua_lcommon.h>

#include <draw/Animation.h>
#include <draw/DirectionalDrawable.h>
#include <draw/Drawable.h>
#include <draw/Colour.h>
#include <draw/colour_constants.h>
#include <draw/display.h>
#include <draw/draw.h>
#include <draw/Image.h>
#include <draw/Font.h>

#include <draw/lua_ldraw.h>

#include "../../draw/parse_drawable.h"
#include "../../data/parse_context.h"

static lua_State* L;

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
	Timer timer;
	int frames = 0;

	while (1) {
		ldraw::Font fpsfont("res/sample.ttf", 40);
		frames += 1;
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (!handle_event(&event)) {
				return; // Exit draw loop
			}
		}
		ldraw::display_draw_start();
		draw_func();
		double seconds = timer.get_microseconds() / 1000.0 / 1000.0;
		fpsfont.drawf(ldraw::DrawOptions(COL_GOLD).origin(ldraw::RIGHT_BOTTOM),
				Posf(400, 400), "%d", int(frames / seconds));
		ldraw::display_draw_finish();
		SDL_Delay(5);
	}
}

static void draw_script() {
	ldraw::Font fpsfont("res/sample.ttf", 40);
	luawrap::globals(L)["draw"].push();
	luawrap::call<void>(L);

	fpsfont.drawf(ldraw::DrawOptions(COL_GOLD).origin(ldraw::LEFT_BOTTOM),
			Posf(0, 400), "Lua");
}
static void draw_luascript(lua_State* L, const char* file) {
	lua_safe_dofile(L, file);

	draw_loop(draw_script);
	if (lua_tostring(L,-1)) {
		printf("%s\n", lua_tostring(L,-1));
	}
}

static void string2node(const std::string& str, YAML::Node& root) {
	std::stringstream ss(str);
	YAML::Parser parser(ss);
	parser.GetNextDocument(root);
}

static ldraw::Drawable drawable_parse(const std::string& str) {
	ParseContext context(L);
	YAML::Node root;
	string2node(str, root);
	return parse_drawable(root);
}

static std::vector<ldraw::Drawable> drawablelist_parse(const std::string& str) {
	ParseContext context(L);
	YAML::Node root;
	string2node(str, root);
	return parse_drawable_list(root);
}

static void setup_lua_state() {
	using namespace ldraw;

	L = lua_open();
	luaL_openlibs(L);

	LuaValue globals(L, LUA_GLOBALSINDEX);

	lua_register_ldraw(L, globals);
	lua_register_lcommon(L, globals);

	// Expose additional functions needed for examples
	// This allows us to code the examples fully in lua:
	globals["drawable_parse"].bind_function(drawable_parse);
	globals["drawablelist_parse"].bind_function(drawablelist_parse);
}

int main(int argc, const char** argv) {
	using namespace ldraw;

	if (argc < 2) {
		printf("Requires script file to run as argument! Exiting ...\n");
		exit(1);
	}

	display_initialize("Lanarts Example", Dim(400, 400), false);

	setup_lua_state();

	draw_luascript(L, argv[1]);

	lua_close(L);
}

