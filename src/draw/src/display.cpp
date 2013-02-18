/*
 * display.cpp:
 *  Control display options, and set indicate that you are done drawing.
 */

#include <SDL.h>
#undef GL_GLEXT_VERSION
#include <SDL_opengl.h>
#include <GL/glu.h>

#include <lcommon/geometry.h>
#include <lcommon/perf_timer.h>

#include "display.h"

#include "opengl/gl_extensions.h"

static void gl_set_drawing_area(int x, int y, int w, int h) {
	glViewport(x, y, w, h);

	//Set projection
	glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	glLoadIdentity();

	/* This allows alpha blending of 2D textures with the scene */
	glEnable(GL_BLEND);
	//glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Set up the coordinate system 0 -> w, 0 -> h
	glOrtho(0.0, (GLdouble)w, (GLdouble)h, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Set up sane 2D drawing defaults
static void gl_sdl_initialize(const char* window_name, int w, int h, bool fullscreen) {

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);

	int bpp = SDL_GetVideoInfo()->vfmt->BitsPerPixel;

	/* Set the flags we want to use for setting the video mode */
	Uint32 video_flags = SDL_OPENGL | SDL_DOUBLEBUF;
	if (fullscreen)
		video_flags |= SDL_FULLSCREEN;

	if (SDL_SetVideoMode(w, h, bpp, video_flags) == NULL) {
		fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	/* Set the window manager title bar */
	SDL_WM_SetCaption(window_name, window_name);

	glDisable(GL_TEXTURE_2D);

	if (!gl_set_vsync(false) ) {
		printf("Disabling vsync not supported, please do this in graphics card settings for best performance.\n");
	}

	gl_set_drawing_area(0, 0, w, h);
}

static void gl_set_fullscreen(bool fullscreen) {
	Uint32 video_flags = SDL_OPENGL | SDL_DOUBLEBUF;
	if (fullscreen) {
		video_flags |= SDL_FULLSCREEN;
	}

	const SDL_VideoInfo* vid_info = SDL_GetVideoInfo();
	int w = vid_info->current_w, h = vid_info->current_h;
	int bpp;

	/*detect the display depth */
	if (vid_info->vfmt->BitsPerPixel <= 8) {
		bpp = 8;
	} else {
		bpp = 16; /* More doesn't seem to work */
	}

	if (SDL_SetVideoMode(w, h, bpp, video_flags) == NULL) {
		fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}
}

void ldraw::display_initialize(const char* window_name,
		const Size& draw_area_size, bool fullscreen) {
	//TODO: Allow passing video flags, esp. SDL_NOFRAME
	gl_sdl_initialize(window_name, draw_area_size.w, draw_area_size.h,
			fullscreen);
}

void ldraw::display_set_fullscreen(bool fullscreen) {
	gl_set_fullscreen(fullscreen);
}

void ldraw::display_set_drawing_region(const BBoxF & bbox) {
	gl_set_drawing_area(bbox.x1, bbox.y1, bbox.width(), bbox.height());
}

bool ldraw::display_is_fullscreen() {
	const SDL_Surface* surface = SDL_GetVideoSurface();
	return (surface->flags & SDL_FULLSCREEN);
}

void ldraw::display_draw_start() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void ldraw::display_draw_finish() {
	perf_timer_begin("SDL_GL_SwapBuffers");
	SDL_GL_SwapBuffers();
	perf_timer_end("SDL_GL_SwapBuffers");
}

Size ldraw::display_size() {
	const SDL_Surface* surface = SDL_GetVideoSurface();
	return Size(surface->w, surface->h);
}

