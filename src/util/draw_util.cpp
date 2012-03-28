#include "draw_util.h"
#include <cmath>
#include <SDL_opengl.h>
#include <cstdio>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void gl_draw_circle(float x, float y, float radius, const Colour& clr, bool outline) {
	if (outline)
		glBegin(GL_LINE_STRIP);
	else
		glBegin(GL_POLYGON);
	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	for (int i = 0; i < 360; i++) {
		float degInRad = i * DEG2RAD;
		glVertex2f(x + cos(degInRad) * radius, y + sin(degInRad) * radius);
	}

	glEnd();
}
void gl_draw_rectangle(int x, int y, int w, int h, const Colour& clr) {
	int x2 = x + w, y2 = y + h;

	glBegin(GL_QUADS);
	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	//Draw our four points, clockwise.
	glVertex2i(x, y);
	glVertex2i(x2, y);
	glVertex2i(x2, y2);
	glVertex2i(x, y2);

	glEnd();
}

void gl_draw_statbar( int x, int y, int w, int h,
		int minstat, int maxstat,
		const Colour& front, const Colour& back) {
	int hp_width = (w * minstat) / maxstat;
	gl_draw_rectangle(x, y, w, h, back);
	if (hp_width > 0)
		gl_draw_rectangle(x, y, hp_width, h, front);
}
void gl_draw_rectangle_parts(int x, int y, int w, int h, int sub_parts,
		char* flags, const Colour& clr) {
	glBegin(GL_QUADS);
	int xincr = w / sub_parts, yincr = h / sub_parts;
	for (int ysub = 0; ysub < sub_parts; ysub++) {
		for (int xsub = 0; xsub < sub_parts; xsub++, flags++) {
			glColor4ub(clr.r, clr.g, clr.b, clr.a);
			if (*flags)
				continue;
			int sx = x + xincr * xsub, ex = sx + xincr;
			int sy = y + yincr * ysub, ey = sy + yincr;
			//Draw our four points, clockwise.
			glVertex2i(sx, sy);
			glVertex2i(ex, sy);
			glVertex2i(ex, ey);
			glVertex2i(sx, ey);
		}
	}
	glEnd();
}

///So while glRasterPos won't let us set the raster position using
///window coordinates, these hacky functions will let us move the current raster
///position a given delta x or y.
inline void move_raster(int x, int y) {
	glBitmap(0, 0, 0, 0, x, y, NULL);
}

///Much like Nehe's glPrint function, but modified to work
///with freetype fonts.
void gl_printf(const font_data &ft_font, const Colour& colour, float x, float y,
		const char *fmt, ...) {
	//	float h=ft_font.h/.63f;						//We make the height about 1.5* that of

	char text[256]; // Holds Our String
	va_list ap; // Pointer To List Of Arguments

	if (fmt == NULL) // If There's No Text
		*text = 0; // Do Nothing

	else {
		va_start(ap, fmt);
		// Parses The String For Variables
		vsprintf(text, fmt, ap); // And Converts Symbols To Actual Numbers
		va_end(ap);
		// Results Are Stored In Text
	}
	int textlen = strlen(text);
	for (int i = 0; i < textlen; i++)
		if (text[i] == '\n')
			text[i] = '\0';
	for (char* iter = text; iter < text + textlen;
			iter += strlen(iter) + 1, y += ft_font.h + 1) {
		glRasterPos2f(0, 0);
		//Trick to avoid drawing problems outside of view region:
		move_raster(x, -y - ft_font.h);

		glPushAttrib(GL_CURRENT_BIT | GL_PIXEL_MODE_BIT | GL_ENABLE_BIT);
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glDisable(GL_TEXTURE_2D);

		//we'll be nice people and save the old pixel unpack alignment-
		//while setting the unpack allignment to one couldn't possibly
		//hurt anyone else's pixel drawing, it might slow it down.
		GLint old_unpack;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &old_unpack);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glColor3ub(colour.r, colour.g, colour.b);
		float color[4];
		glGetFloatv(GL_CURRENT_COLOR, color);

		glPixelTransferf(GL_RED_SCALE, color[0]);
		glPixelTransferf(GL_GREEN_SCALE, color[1]);
		glPixelTransferf(GL_BLUE_SCALE, color[2]);
		glPixelTransferf(GL_ALPHA_SCALE, color[3]);

		for (int i = 0; iter[i]; i++) {
			const char_data &cdata = *ft_font.data[iter[i]];

			move_raster(cdata.left, cdata.move_up);

			glDrawPixels(cdata.w, cdata.h, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
					cdata.data);

			move_raster(cdata.advance - cdata.left, -cdata.move_up);

		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, old_unpack);
		glPopAttrib();
	}
}
