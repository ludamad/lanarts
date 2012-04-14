/*
 * font_util.h
 *
 *  Created on: 2011-11-02
 *      Author: 100397561
 */

#ifndef FONT_UTIL_H_
#define FONT_UTIL_H_

#include <GL/glu.h>
#undef GL_GLEXT_VERSION
#include <ft2build.h>
#include <freetype/freetype.h>
#include "../display/GLImage.h"

///This is the datastructure that I'm using to store everything I need
///to render a character glyph in opengl.
struct char_data {
	int w,h;
	int advance;
	int left;
	int move_up;
	unsigned char * data;
	GLImage img;

	char_data(char ch, FT_Face face);
	~char_data();
};
//This holds all of the information related to any
//freetype font that we want to create.
struct font_data {
	char_data* data[128];
	float h;			///< Holds the height of the font
};
//The init function will create a font of
//of the height h from the file fname.
void init_font(font_data* fd, const char * fname, unsigned int h);
void release_font(font_data* fd);




#endif /* FONT_UTIL_H_ */
