#include <cmath>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#undef GL_GLEXT_VERSION
#include <SDL_opengl.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "../data/sprite_data.h"

#include "../world/GameView.h"

#include "display.h"
#include "font.h"
#include "GLImage.h"

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

const int UNBOUNDED = 10000;

/*Splits up strings, respecting space boundaries*/
static std::vector<int> split_up_string(const font_data& font, int max_width,
		const char* text) {
	std::vector<int> line_splits;
	int last_space = 0, ind = 0;
	int width = 0, width_since_space = 0;
	unsigned char c;

	while ((c = text[ind]) != '\0') {
		char_data &cdata = *font.data[c];
		width += cdata.advance;
		width_since_space += cdata.advance;

		if (isspace(c)) {
			last_space = ind;
			width_since_space = 0;
		}

		if (width > max_width) {
			line_splits.push_back(last_space);
			width = width_since_space;
		}
		ind++;
	}

	return line_splits;
}

//Pos gl_printf_bounded(const font_data& font, const Colour& colour,
//		const BBox& bounds, const char* text) {
//	int textlen = strlen(text);
//	for (int i = 0; i < textlen; i++)
//		if (text[i] == '\n')
//			text[i] = '\0';
//
//	Pos offset(0, 0);
//	int len = 0;
//	const char* iter = text;
//	while (iter < text + textlen) {
//		int len = 0;
//		offset.y += font.h;
//		for (int i = 0; iter[i]; i++) {
//			unsigned char chr = iter[i];
//			char_data &cdata = *font.data[chr];
//			len += cdata.advance;
//			gl_draw_image(cdata.img, bounds.x1 + len - (cdata.advance - cdata.left),
//					bounds.y1 + offset.y - cdata.move_up, colour);
//		}
//		offset.x = std::max(len, offset.x);
//		offset.y += 1;
//
//		iter += strlen(iter) + 1;
//	}
//	return offset;
//}

/* printf-like function that draws to the screen, returns width of formatted string*/
void gl_draw_sprite_entry(const GameView& view, SpriteEntry& entry, int x,
		int y, float dx, float dy, int steps, const Colour& c) {
	float PI = 3.1415921;
	GLimage* img;
	if (entry.type == SpriteEntry::DIRECTIONAL) {
		float direction = PI*2.5 + atan2(dy, dx);
		int nimgs = entry.images.size();
		float bucket_size = PI*2 / nimgs;
		int bucket = round(direction / bucket_size);
		bucket = bucket % nimgs;
		img = &entry.img(bucket);
	} else if (entry.type == SpriteEntry::ANIMATED) {
		img = &entry.img();
	}
	gl_draw_image(view, *img, x, y, c);
}

Pos gl_printf(const font_data& font, const Colour& colour, float x, float y,
		const char *fmt, ...) {
	char text[512];
	va_list ap;

	va_start(ap, fmt);
#ifdef __GNUC__
	vsnprintf(text, 512, fmt, ap);
#else
	//TODO, add visual studio #if block
	vsprintf(text, fmt, ap);
#endif
	va_end(ap);

	int textlen = strlen(text);
	for (int i = 0; i < textlen; i++)
		if (text[i] == '\n')
			text[i] = '\0';
	Pos offset(0, 0);
	int len = 0;
	for (char* iter = text; iter < text + textlen;
			iter += strlen(iter) + 1, len = 0) {
		offset.y += font.h;
		for (int i = 0; iter[i]; i++) {
			unsigned char chr = iter[i];
			char_data &cdata = *font.data[chr];
			len += cdata.advance;
			offset.x = std::max(len, offset.x);
			gl_draw_image(cdata.img, x + len - (cdata.advance - cdata.left),
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

	glVertex2i(x - linewidth, y);
	glVertex2i(x2 - linewidth, y);
	glVertex2i(x2 - linewidth, y2 - linewidth);
	glVertex2i(x, y2 - linewidth);
	glVertex2i(x, y);

	glEnd();

	if (linewidth != 1)
		glLineWidth(1);
}

void gl_draw_line(int x1, int y1, int x2, int y2, const Colour& clr,
		int linewidth) {

	if (linewidth != 1)
		glLineWidth(linewidth);

	glBegin(GL_LINES);
	glColor4ub(clr.r, clr.g, clr.b, clr.a);

	glVertex2i(x1, y1);
	glVertex2i(x2, y2);

	glEnd();

	if (linewidth != 1)
		glLineWidth(1);
}

void gl_draw_statbar(const GameView& view, int x, int y, int w, int h,
		int min_stat, int max_stat, const Colour& front, const Colour& back) {
	gl_draw_statbar(x - view.x, y - view.y, w, h, min_stat, max_stat, front,
			back);
}

