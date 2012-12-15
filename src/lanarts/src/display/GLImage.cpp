/*
 * GLImage.cpp:
 *  A convenient OpenGL image wrapper
 */

#include "lanarts_defines.h"
#include <SDL.h>
#include <SDL_opengl.h>

//Surpress some multiple defintion warnings:
#undef GL_GLEXT_VERSION
#include <SDL_image.h>
#include <GL/glu.h>

#include "GLImage.h"

/* Utility function for power-of-two texture creation */
static int power_of_two(int input) {
	int value = 1;

	while (value < input) {
		value <<= 1;
	}
	return value;
}

/* Utility function for conversion between SDL surfaces and GL surfaces */
static GLuint SDL_GL_LoadTexture(SDL_Surface *surface, GLfloat *texcoord) {
	GLuint texture;
	int w, h;
	SDL_Surface *image;
	SDL_Rect area;
	Uint32 saved_flags;
	Uint8 saved_alpha;

	/* Use the surface width and height expanded to powers of 2 */
	w = power_of_two(surface->w);
	h = power_of_two(surface->h);
	texcoord[0] = 0.0f; /* Min X */
	texcoord[1] = 0.0f; /* Min Y */
	texcoord[2] = (GLfloat)surface->w / w; /* Max X */
	texcoord[3] = (GLfloat)surface->h / h; /* Max Y */

	image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF
#endif
			);
	if (image == NULL) {
		return 0;
	}

	/* Save the alpha blending attributes */
	saved_flags = surface->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
	saved_alpha = surface->format->alpha;
	if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA) {
		SDL_SetAlpha(surface, 0, 0);
	}

	/* Copy the surface into the GL texture image */
	area.x = 0;
	area.y = 0;
	area.w = surface->w;
	area.h = surface->h;
	SDL_BlitSurface(surface, &area, image, &area);

	/* Restore the alpha blending attributes */
	if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA) {
		SDL_SetAlpha(surface, saved_flags, saved_alpha);
	}

	/* Create an OpenGL texture for the image */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			image->pixels);
	SDL_FreeSurface(image); /* No longer needed */

	return texture;
}

void init_GL_Image(GLimage& img, const char* fname) {
	SDL_Surface* image;
	GLfloat texcoord[4];

	if (fname == NULL || img.texture != 0)
		return;
	/* Load the image using SDL_image library */
	image = IMG_Load(fname);
	if (image == NULL) {
		printf("Image '%s' could not be loaded\n", fname);
		return;
	}
	img.filename = fname;
	img.width = image->w;
	img.height = image->h;

	/* Convert the image into an OpenGL texture */
	img.texture = SDL_GL_LoadTexture(image, texcoord);
	if (!img.texture)
		printf("Texture from image '%s' could not be loaded\n", fname);
	img.texw = texcoord[2];
	img.texh = texcoord[3];

	/* We don't need the original image anymore */
	SDL_FreeSurface(image);
}

GLimage::GLimage(const GLimage& img) :
		filename(img.filename), height(img.height), width(img.width), texw(
				img.texw), texh(img.texh), texture(0) {
	if (img.texture != 0){
	LANARTS_ASSERT(img.texture == 0);
	}
}

GLimage::~GLimage() {
	if (texture) {
		glDeleteTextures(1, (GLuint*)&texture);
	}
}

void GLimage::init() {

	SDL_Surface* image;
	GLfloat texcoord[4];

	if (filename.empty() || texture != 0)
		return;
	/* Load the image using SDL_image library */
	image = IMG_Load(filename.c_str());
	if (image == NULL) {
		printf("Image '%s' could not be loaded\n", filename.c_str());
		return;
	}

	width = image->w;
	height = image->h;

	/* Convert the image into an OpenGL texture */
	texture = SDL_GL_LoadTexture(image, texcoord);

	if (!texture) {
		printf("Texture from image '%s' could not be loaded\n",
				filename.c_str());
	}

	texw = texcoord[2];
	texh = texcoord[3];

	/* We don't need the original image anymore */
	SDL_FreeSurface(image);
}

