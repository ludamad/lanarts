/*
 * GLImage.h:
 *  A convenient OpenGL image wrapper
 */

#ifndef GLIMAGE_H_
#define GLIMAGE_H_

#include <string>

#include <SDL.h>
#include <SDL_opengl.h>
#undef GL_GLEXT_VERSION

/* can be copy constructed so long as it has not been init'd yet */
struct GLimage {
	GLimage(const std::string filename = std::string()) :
			filename(filename) {
		texture = 0;
	}
	/* asserts img.texture == 0 */
	GLimage(const GLimage& img);
	~GLimage();

	void init();

	std::string filename;
	int width, height;
	float texw, texh;
	GLuint texture;
};

#endif /* GLIMAGE_H_ */
