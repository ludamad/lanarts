/*
 * Display.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: 100397561
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

#include "display.h"
#include "GLImage.h"

#include "../util/math_util.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <algorithm>
#include "../world/GameView.h"

void gl_draw_image(GLImage* img, int x, int y, const Colour& c) {
	if (img->width == 0 || img->height == 0) return;
	int x2 = x + img->width, y2 = y + img->height;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, img->texture);

	glColor4ub(c.r, c.g, c.b, c.a);

	glBegin(GL_QUADS);
	//Draw our four points, clockwise.
	glTexCoord2f(0, 0);
	glVertex2i(x, y);

	glTexCoord2f(img->texw, 0);
	glVertex2i(x2, y);

	glTexCoord2f(img->texw, img->texh);
	glVertex2i(x2, y2);

	glTexCoord2f(0, img->texh);
	glVertex2i(x, y2);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Don't use glBindTexture(GL_TEXTURE_2D, NULL);
}
void gl_draw_image(const GameView& view, GLImage* img, int x, int y){
	gl_draw_image(img, x - view.x, y - view.y);
}


void update_display() {
	SDL_GL_SwapBuffers();
}

void SDL_GL_diagnostics() {
	printf("SDL_GL_diagnostics()::\n");
	int value;
	printf("Screen BPP: %d\n", SDL_GetVideoSurface()->format->BitsPerPixel);
	printf("\n");
	printf("Vendor     : %s\n", glGetString(GL_VENDOR));
	printf("Renderer   : %s\n", glGetString(GL_RENDERER));
	printf("Version    : %s\n", glGetString(GL_VERSION));
	printf("Extensions : %s\n", glGetString(GL_EXTENSIONS));
	printf("\n");

	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
	printf("SDL_GL_RED_SIZE: %d\n", value);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value);
	printf("SDL_GL_GREEN_SIZE: %d\n", value);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
	printf("SDL_GL_BLUE_SIZE: %d\n", value);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
	printf("SDL_GL_DEPTH_SIZE:  %d\n", value);
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &value);
	printf("SDL_GL_DOUBLEBUFFER: %d\n", value);
	fflush(stdout);
}
void init_sdl_gl(bool fullscreen, int w, int h) {

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return;
	}
	int bpp;
	Uint32 video_flags;

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

	/*detect the display depth */
	if (SDL_GetVideoInfo()->vfmt->BitsPerPixel <= 8) {
		bpp = 8;
	} else {
		bpp = 16; /* More doesn't seem to work */
	}

	/* Set the flags we want to use for setting the video mode */
	video_flags = SDL_OPENGL;
	if (fullscreen)
		video_flags |= SDL_FULLSCREEN;

	if (SDL_SetVideoMode(w, h, bpp, video_flags) == NULL) {
		fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	/* Set the window manager title bar */
	SDL_WM_SetCaption("RPG", "rpg");

	glDisable(GL_TEXTURE_2D);
}

void gl_set_drawing_area(int x, int y, int w, int h) {

	glViewport(x, y, w, h);

	//Set projection
	glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	glLoadIdentity();

	//glOrtho(0.0, (GLdouble)w, (GLdouble)h, 0.0, 0.0, 1.0);
	//glOrtho(0.0, (GLdouble)w, 0.0, (GLdouble)h, 0.0, 1.0);

	/* This allows alpha blending of 2D textures with the scene */
	glEnable(GL_BLEND);
	//glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_CULL_FACE);
	//glPushAttrib(GL_ENABLE_BIT);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glShadeModel(GL_SMOOTH);
	//Clear projection

	//Set up the coordinate system 0 -> w, 0 -> h
	glOrtho(0.0, (GLdouble) w, (GLdouble) h, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//SDL_GL_diagnostics();

	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	//update_display();
}

void gl_image_from_bytes(GLImage* img, int w, int h, char* data, int type) {
	bool was_init = img->texture;
	// allocate a texture name
	if (!was_init)
		glGenTextures(1, &img->texture);

	int ptw = power_of_two(w), pth = power_of_two(h);
	ptw = std::max(4, ptw);
	pth = std::max(4, pth);

	img->width = w, img->height = h;
	img->texw = w / ((float) ptw);
	img->texh = h / ((float) pth);
	if (w == 0 || h == 0) return;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, img->texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if (!was_init)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ptw, pth, 0, type,
				GL_UNSIGNED_BYTE, NULL);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, type, GL_UNSIGNED_BYTE,
			data);
	glDisable(GL_TEXTURE_2D);
}
