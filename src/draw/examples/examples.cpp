/*
 * shapes_example.cpp:
 *  Exercises the shape drawing methods
 */

#include <SDL.h>
#include <GL/glu.h>

#include "../Colour.h"

#include "../colour_constants.h"

#include "../display.h"
#include "../draw.h"
#include "../opengl/GLImage.h"

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
		if (keycode == SDLK_ESCAPE) {
			return false;
		}
		if (keycode == SDLK_RETURN) {
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
		ldraw::draw_start();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (!handle_event(&event)) {
				return; // Exit draw loop
			}
		}
		draw_func();
		ldraw::draw_finish();
	}
}

static void draw_shapes() {
	ldraw::draw_rectangle(COL_LIGHT_RED, BBoxF(100, 100, 300, 300));
	ldraw::draw_circle(COL_LIGHT_BLUE, Posf(200, 200), 100);
}

GLImage image;

static void draw_images() {
	BBox box(0, 0, 10, 10);
	FOR_EACH_BBOX(box, x, y) {
		image.draw(Pos(x * 40, y * 40));
	}
}

int main() {
	ldraw::display_initialize(__FILE__, Dim(400, 400), false);
	image.initialize("sample.png");
	draw_loop(draw_shapes);
	draw_loop(draw_images);
}
