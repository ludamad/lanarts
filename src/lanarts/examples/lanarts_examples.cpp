/*
 * lanarts_examples.cpp:
 *  Exercises some of the hard-to-unit test code
 */

#include <SDL.h>
#include <GL/glu.h>

#include <lua.hpp>

#include <yaml-cpp/yaml.h>

#include <lcommon/mathutil.h>
#include <lcommon/Timer.h>

#include <luawrap/LuaValue.h>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/calls.h>

#include <lcommon/lua_lcommon.h>

#include <ldraw/Animation.h>
#include <ldraw/DirectionalDrawable.h>
#include <ldraw/Drawable.h>
#include <ldraw/Colour.h>
#include <ldraw/colour_constants.h>
#include <ldraw/display.h>
#include <ldraw/draw.h>
#include <ldraw/Image.h>
#include <ldraw/Font.h>

#include <ldraw/lua_ldraw.h>

#include "draw/parse_drawable.h"
#include "data/parse_context.h"

#include "gamestate/GameState.h"

#include "lua_api/lua_newapi.h"

static lua_State* L;

typedef void (*DrawFunc)();

static void draw_loop(DrawFunc draw_func) {
	Timer timer;
	int frames = 0;

	while (1) {
		frames += 1;

		if (!lua_api::gamestate(L)->update_iostate()) {
			break;
		}

		ldraw::display_draw_start();
		draw_func();
		double seconds = timer.get_microseconds() / 1000.0 / 1000.0;

		ldraw::display_draw_finish();
		SDL_Delay(5);
	}
}

static void draw_script() {
	luawrap::globals(L)["draw"].push();
	luawrap::call<void>(L);
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

	GameSettings settings;

	//GameState claims ownership of the passed lua_State*
	GameState* gs = new GameState(settings, L, 200, 200);
	lua_api::register_api(gs, L);

	LuaValue globals(L, LUA_GLOBALSINDEX);

	// Expose additional functions needed for examples
	// This allows us to code the examples fully in lua:
	globals["drawable_parse"].bind_function(drawable_parse);
	globals["drawablelist_parse"].bind_function(drawablelist_parse);
}

// Must be char** argv to play nice with SDL on windows!
int main(int argc, char** argv) {
	using namespace ldraw;

	if (argc < 2) {
		printf("Requires script file to run as argument! Exiting ...\n");
		exit(1);
	}

	display_initialize("Lanarts Example", Dim(640, 480), false);

	setup_lua_state();

	draw_luascript(L, argv[1]);

	lua_close(L);
}
