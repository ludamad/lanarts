#include "display.h"
#include <cmath>
#undef GL_GLEXT_VERSION
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

#include "GLImage.h"
#include "font.h"
#include "../world/GameView.h"

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void gl_draw_circle(float x, float y, float radius, const Colour& clr,
		bool outline) {
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

	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	//Draw our four points, clockwise.
	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x2, y);
	glVertex2i(x2, y2);
	glVertex2i(x, y2);

	glEnd();
}

void gl_draw_statbar(int x, int y, int w, int h, int minstat, int maxstat,
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

///Much like Nehe's glPrint function, but modified to work
///with freetype fonts.
/*returns width of formatted string*/
Pos gl_printf(const font_data &ft_font, const Colour& colour, float x, float y,
		const char *fmt, ...) {
	//	float h=ft_font.h/.63f;						//We make the height about 1.5* that of
	char text[512]; // Holds Our String
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
	Pos offset(0, 0);
	int len = 0;
	for (char* iter = text; iter < text + textlen;
			iter += strlen(iter) + 1, len = 0) {
		offset.y += ft_font.h;
		for (int i = 0; iter[i]; i++) {
			unsigned char chr = iter[i];
			char_data &cdata = *ft_font.data[chr];
			len += cdata.advance;
			offset.x = std::max(len, offset.x);
			gl_draw_image(&cdata.img, x + len - (cdata.advance - cdata.left),
					y + offset.y - cdata.move_up, colour);
		}
		offset.y += 1;
	}
	return offset;
}

void gl_draw_circle(const GameView& view, float x, float y, float radius,
		const Colour& colour, bool outline) {
	gl_draw_circle(x - view.x, y - view.y, radius, colour, outline);
}
void gl_draw_rectangle(const GameView& view, int x, int y, int w, int h,
		const Colour& colour) {
	gl_draw_rectangle(x - view.x, y - view.y, w, h, colour);
}

void gl_draw_rectangle_outline(int x, int y, int w, int h, const Colour& clr,
		int linewidth) {
	int x2 = x + w, y2 = y + h;

	if (linewidth != 1)
		glLineWidth(linewidth);

	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	//Draw our four points, clockwise.

	glBegin(GL_LINE_STRIP);

	glVertex2i(x+linewidth, y+linewidth);
	glVertex2i(x2, y+linewidth);
	glVertex2i(x2, y2);
	glVertex2i(x+linewidth, y2);
	glVertex2i(x+linewidth, y);

	glEnd();

	if (linewidth != 1)
		glLineWidth(1);
}

void gl_draw_statbar(const GameView& view, int x, int y, int w, int h,
		int min_stat, int max_stat, const Colour& front, const Colour& back) {
	gl_draw_statbar(x - view.x, y - view.y, w, h, min_stat, max_stat, front,
			back);
}

