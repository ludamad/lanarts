/*
 * GLImage.cpp:
 *  A convenient OpenGL image wrapper
 */

#include <common/lcommon_defines.h>
#include <common/math.h>

#include <SDL.h>
#include <SDL_opengl.h>

//Surpress some multiple defintion warnings:
#undef GL_GLEXT_VERSION
#include <SDL_image.h>
#include <GL/glu.h>

#include "../ldraw_assert.h"

#include "GLImage.h"

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

GLImage::~GLImage() {
	if (texture) {
		glDeleteTextures(1, (GLuint*)&texture);
	}
}

void GLImage::initialize(const std::string& filename) {

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

static void gl_subimage_from_bytes(GLImage& img, const BBox& region, char* data,
		int type) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, img.texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexSubImage2D(GL_TEXTURE_2D, 0, region.x1, region.y1, region.width(),
			region.height(), type, GL_UNSIGNED_BYTE, data);
	glDisable(GL_TEXTURE_2D);
}

static void gl_image_from_bytes(GLImage& img, const Dim& size, char* data,
		int type) {
	bool was_init = img.texture;
	// allocate a texture name
	if (!was_init)
		glGenTextures(1, &img.texture);

	int ptw = power_of_two(size.w), pth = power_of_two(size.h);
	ptw = std::max(4, ptw);
	pth = std::max(4, pth);

	img.width = size.w, img.height = size.h;
	img.texw = size.w / ((float)ptw);
	img.texh = size.h / ((float)pth);
	if (size.w == 0 || size.h == 0)
		return;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, img.texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if (!was_init)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ptw, pth, 0, type,
				GL_UNSIGNED_BYTE, NULL);
	if (data) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.w, size.h, type,
				GL_UNSIGNED_BYTE, data);
	}
	glDisable(GL_TEXTURE_2D);
}

static void gl_draw_image(GLuint texture, const Colour& c, const BBoxF& texbox,
		const BBoxF& imgbox) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glColor4ub(c.r, c.g, c.b, c.a);

	glBegin(GL_QUADS);
	//Draw our four points, clockwise.
	glTexCoord2f(texbox.x1, texbox.y1);
	glVertex2i(imgbox.x1, imgbox.y1);

	glTexCoord2f(texbox.x2, texbox.y1);
	glVertex2i(imgbox.x2, imgbox.y1);

	glTexCoord2f(texbox.x2, texbox.y2);
	glVertex2i(imgbox.x2, imgbox.y2);

	glTexCoord2f(texbox.x1, texbox.y2);
	glVertex2i(imgbox.x1, imgbox.y2);

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void GLImage::subimage_from_bytes(const BBox& region, char* data, int type) {
	gl_subimage_from_bytes(*this, region, data, type);
}

void GLImage::image_from_bytes(const Dim& size, char* data, int type) {
	gl_image_from_bytes(*this, size, data, type);
}

void GLImage::draw(const ldraw::DrawOptions& options, const Posf& pos) {
	BBoxF draw_region(0, 0, width, height);
	if (!options.draw_region.empty()) {
		LDRAW_ASSERT(options.draw_region.x1 > 0 && options.draw_region.y1 > 0);
		LDRAW_ASSERT(options.draw_region.x2 <= width);
		LDRAW_ASSERT(options.draw_region.y2 <= height);

		draw_region = options.draw_region;
	}
	if (draw_region.empty()) {
		return;
	}
	gl_draw_image(texture, options.draw_colour,
			draw_region.scaled(texw / width, texh / height),
			draw_region.translated(pos));
}
