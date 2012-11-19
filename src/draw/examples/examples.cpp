/*
 * shapes_example.cpp:
 *  Exercises the shape drawing methods
 */

#include <SDL.h>
#include <GL/glu.h>

#include <SLB/Script.hpp>
#include <SLB/LuaCall.hpp>
#include <SLB/Manager.hpp>

#include <common/Timer.h>
#include <common/lua/lua_lcommon.h>
#include <common/lua/LuaValue.h>

#include <common/math.h>

#include <lua/lualib.h>

#include "../lua/lua_ldraw.h"

#include "../Colour.h"

#include "../colour_constants.h"

#include "../display.h"
#include "../draw.h"
#include "../Image.h"
#include "../Animation.h"
#include "../DirectionalDrawable.h"
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
	ldraw::Font fpsfont("sample.ttf", 40);
	Timer timer;
	int frames = 0;
	while (1) {
		frames += 1;
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (!handle_event(&event)) {
				return; // Exit draw loop
			}
		}
		ldraw::draw_start();
		draw_func();
		double seconds = timer.get_microseconds() / 1000.0 / 1000.0;
		fpsfont.drawf(ldraw::DrawOptions(COL_GOLD).origin(ldraw::RIGHT_BOTTOM),
				Posf(400, 400), "%d", int(frames / seconds));
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
		image.draw(Posf(x * 40, y * 40));
	}
}

ldraw::Font font;

static void draw_text() {
	using namespace ldraw;

	font.draw(COL_WHITE, Pos(0, 0), "Hello World!");
	font.draw_wrapped(DrawOptions().origin(CENTER).colour(COL_PALE_RED),
			Pos(200, 200), 250,
			"This text is wrapped because it's sort of long.");
}

ldraw::Drawable arrow;

static void draw_directional() {
	using namespace ldraw;
	Posf center(200, 200);
	BBox box(0, 0, 10, 10);
	FOR_EACH_BBOX(box, x, y) {
		Posf pos(x * 40 + 20, y * 40 + 20);
		float dir = compute_direction(pos, center);
		arrow.draw(DrawOptions().angle(dir).origin(CENTER), pos);
	}
}

ldraw::Drawable animation;
float frame = 0.0f;

static void draw_animation() {
	using namespace ldraw;
	Posf center(200, 200);
	frame += 1.0f;
	animation.draw(DrawOptions().origin(CENTER).frame(frame), center);
}

lua_State* L;
SLB::Manager m;

static void draw_script() {
	SLB::LuaCall<void()> drawfunc(L, "draw");
	drawfunc();
}

static void setup_lua_state() {
	L = lua_open();
	m.registerSLB(L);
	luaL_openlibs(L);

	LuaValue globals(L, LUA_GLOBALSINDEX);

	lua_register_ldraw(L, globals);
	lua_register_lcommon(L, globals);

}

int main(int argc, const char** argv) {
	using namespace ldraw;

	display_initialize(__FILE__, Dim(400, 400), false);
	image.initialize("sample.png");
	font.initialize("sample.ttf", 20);

	std::vector<Image> arr_images = split_image(Image("arrows.png"),
			DimF(32, 32));
	arrow = new DirectionalDrawable(arr_images, FLOAT_PI / 2);

	std::vector<Image> anim_images = split_image(Image("animation.png"),
			DimF(480.0f / 6, 120));
	animation = new Animation(anim_images, 0.01f);

	draw_loop(draw_shapes);
	draw_loop(draw_images);
	draw_loop(draw_text);
	draw_loop(draw_directional);
	draw_loop(draw_animation);

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
