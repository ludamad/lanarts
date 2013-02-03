/*
 * lanarts_examples.cpp:
 *  Exercises some of the hard-to-unit test code
 */

#include <cstdio>
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
#include <lcommon/fatal_error.h>

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

static void draw_luascript(lua_State* L, const char* file) {
	lua_safe_dofile(L, file);
	lua_api::globals_set_mutability(L, false);

	luawrap::globals(L)["main"].push();
	luawrap::call<void>(L);

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

int read_eval_print(lua_State *L);

static void setup_lua_state() {
	using namespace ldraw;

	L = lua_api::create_luastate();
	lua_api::add_search_path(L, "res/?.lua;res/start_menu/?.lua");

	GameSettings settings;

	//GameState claims ownership of the passed lua_State*
	GameState* gs = new GameState(settings, L);
	lua_api::register_api(gs, L);

	LuaValue globals(L, LUA_GLOBALSINDEX);

	// Expose additional functions needed for examples
	// This allows us to code the examples fully in lua:
	globals["drawable_parse"].bind_function(drawable_parse);
	globals["drawablelist_parse"].bind_function(drawablelist_parse);
	globals["read_eval_print"].bind_function(read_eval_print);
}

// Must be char** argv to play nice with SDL on windows!
int main(int argc, char** argv) {
	try {
		using namespace ldraw;

		if (argc < 2) {
			printf("Requires script file to run as argument! Exiting ...\n");
			exit(1);
		}

		Timer timer;
		setup_lua_state();

		ldraw::display_initialize("Lanarts Example Runner", Dim(640, 480));

		draw_luascript(L, argv[1]);

		lua_close(L);
	} catch (const __FatalError& err) {
		printf("Fatal error, exiting ... \n");
	}
}
