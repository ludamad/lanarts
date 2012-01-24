/*
 * GLImage.h
 *
 *  Created on: Jun 9, 2011
 *      Author: 100397561
 */

#ifndef GLIMAGE_H_
#define GLIMAGE_H_

#include <SDL.h>
#include <SDL_opengl.h>

struct GLImage{
	const char* filename;
	int width, height;
	float texw, texh;
	GLuint texture;
	GLImage(const char* fname = NULL){
		filename = fname;
		texture = 0;
	}
	~GLImage();
};

GLuint SDL_GL_LoadTexture(SDL_Surface *surface, GLfloat *texcoord);
void init_GL_Image(GLImage* img, const char* fname);

#endif /* GLIMAGE_H_ */
