/*
 * lua_ldraw_lib.cpp:
 *  Bindings for the ldraw library in lua.
 */

#include <lua.hpp>

#include <lcommon/Timer.h>
#include <lcommon/math_util.h>

#include <luawrap/LuaValue.h>
#include <luawrap/functions.h>

#include <lcommon/lua_geometry.h>
#include <lcommon/lua_lcommon.h>

#include "colour_constants.h"
#include "draw.h"
#include "display.h"
#include "Image.h"
#include <SDL_opengl.h>

#include "lua_image.h"
#include "lua_font.h"
#include "lua_colour.h"
#include "lua_drawable.h"
#include "lua_drawoptions.h"

#include "lua_ldraw.h"

static int lua_draw_rectangle_outline(lua_State* L) {
	using namespace ldraw;
	using namespace luawrap;
	draw_rectangle_outline(get<Colour>(L, 1), get<BBox>(L, 2), get_defaulted(L, 3, 1.0f));
	return 0;
}

static bool handle_event(SDL_Event* event) {
	SDL_Keycode keycode = event->key.keysym.sym;
	SDL_Keymod keymod = (SDL_Keymod) event->key.keysym.mod;

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

static void draw_loop(LuaValue draw_func) {
	int frames = 0;
	while (1) {
		frames += 1;
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (!handle_event(&event)) {
				return; // Exit draw loop
			}
		}
		ldraw::display_draw_start();
                draw_func.push();
                luawrap::call<void>(draw_func.luastate(), frames); 
		ldraw::display_draw_finish();
		SDL_Delay(5);
	}
}

namespace ldraw {
void lua_register_ldraw(lua_State* L, const LuaValue& module, bool register_lcommon) {
	using namespace ldraw;

	if (register_lcommon) {
		lua_register_lcommon(L);
	}

	module["draw_rectangle"].bind_function(draw_rectangle);
	module["draw_circle"].bind_function(draw_circle);
	module["draw_circle_outline"].bind_function(draw_circle_outline);
	module["draw_rectangle_outline"].bind_function(lua_draw_rectangle_outline);
	module["draw_line"].bind_function(draw_line);
	module["initialize"].bind_function(display_initialize);
	module["draw_loop"].bind_function(draw_loop);
	module["set_window_region"].bind_function(display_set_window_region);
	module["set_world_region"].bind_function(display_set_world_region);
	module["reset_blend_func"].bind_function(reset_blend_func);

	lua_register_font(L, module);
	lua_register_image(L, module);
	lua_register_drawoptions(L, module);
	lua_register_colour(L, module);
	lua_register_drawables(L, module);
}

}
