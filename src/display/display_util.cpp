/*
 * display_util.cpp:
 *  Image drawing and other drawing related utility functions
 */

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

void gl_draw_statbar(const BBox& bbox, int minstat, int maxstat,
		const Colour& front, const Colour& back) {
	int bar_width = (bbox.width() * minstat) / maxstat;
	gl_draw_rectangle(bbox.x1, bbox.y1, bbox.width(), bbox.height(), back);
	if (bar_width > 0) {
		gl_draw_rectangle(bbox.x1, bbox.y1, bar_width, bbox.height(), front);
	}
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

/*Splits up strings, respecting space boundaries & returns maximum width */
static int process_string(const font_data& font, const char* text,
		int max_width, std::vector<int>& line_splits) {
	int last_space = 0, ind = 0;
	int largest_width = 0;
	int width = 0, width_since_space = 0;
	unsigned char c;

	while ((c = text[ind]) != '\0') {
		char_data& cdata = *font.data[c];
		width += cdata.advance;
		width_since_space += cdata.advance;

		if (isspace(c)) {
			last_space = ind;
			width_since_space = 0;
		}

		bool overmax = max_width != -1 && width > max_width;
		if (c == '\n' || overmax) {
			line_splits.push_back(last_space);
			largest_width = std::max(width, largest_width);
			width = width_since_space;
		}
		ind++;
	}
	line_splits.push_back(ind);

	largest_width = std::max(width, largest_width);

	return largest_width;
}

void gl_draw_sprite_entry(const GameView& view, SpriteEntry& entry, int x,
		int y, float dx, float dy, int steps, const Colour& c) {
	float PI = 3.1415921;
	GLimage* img;
	if (entry.type == SpriteEntry::DIRECTIONAL) {
		float direction = PI * 2.5 + atan2(dy, dx);
		int nimgs = entry.images.size();
		float bucket_size = PI * 2 / nimgs;
		int bucket = round(direction / bucket_size);
		bucket = bucket % nimgs;
		img = &entry.img(bucket);
	} else if (entry.type == SpriteEntry::ANIMATED) {
		img = &entry.img();
	}
	gl_draw_image(view, *img, x, y, c);
}

void gl_draw_sprite(sprite_id sprite, int x, int y, const Colour& c) {
	GLimage& img = game_sprite_data.at(sprite).img();
	gl_draw_image(img, x, y, c);
}
void gl_draw_sprite(sprite_id sprite, const GameView& view, int x, int y,
		const Colour& c) {
	gl_draw_sprite(sprite, x - view.x, y - view.y, c);
}

#ifndef __GNUC__
#define vsnprintf(text, len, fmt, ap) vsprintf(text, fmt, ap)
#endif

//
/* General gl_print function for others to delegate to */
static Pos gl_print_impl(const font_data& font, const Colour& colour, Pos p,
		int max_width, bool center_x, bool center_y, const char* fmt, va_list ap) {
	char text[512];
	vsnprintf(text, 512, fmt, ap);
	va_end(ap);

	Pos offset(0, 0);

	std::vector<int> line_splits;
	int measured_width = process_string(font, text, max_width, line_splits);

	if (center_x) {
		p.x -= measured_width / 2;
	}
	if (center_y) {
		p.y -= font.h / 2;
	}

	for (int linenum = 0, i = 0; linenum < line_splits.size(); linenum++) {
		int len = 0;
		int eol = line_splits[linenum];

		offset.y += font.h;

		for (; i < eol; i++) {
			unsigned char chr = text[i];
			if (chr == '\n') {
				continue; //skip newline char
			}
			char_data &cdata = *font.data[chr];
			len += cdata.advance;
			gl_draw_image(cdata.img, p.x + len - (cdata.advance - cdata.left),
					p.y + offset.y - cdata.move_up, colour);
		}
		offset.x = std::max(len, offset.x);
		offset.y += 1;
	}
	return offset;
}
/* printf-like function that draws to the screen, returns dimensions of formatted string*/
Pos gl_printf(const font_data& font, const Colour& colour, float x, float y,
		const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), -1, false, false, fmt, ap);
}

/* printf-like function that draws to the screen, returns width of formatted string*/
Pos gl_printf_bounded(const font_data& font, const Colour& colour, float x,
		float y, int max_width, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), max_width, false, false, fmt, ap);
}

/* printf-like function that draws to the screen, returns width of formatted string*/
Pos gl_printf_centered(const font_data& font, const Colour& colour, float x,
		float y, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), -1, true, true, fmt, ap);
}

/* printf-like function that draws to the screen, returns width of formatted string*/
Pos gl_printf_y_centered(const font_data& font, const Colour& colour, float x,
		float y, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), -1, false, true, fmt, ap);
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

void gl_draw_statbar(const GameView& view, const BBox& bbox, int min_stat,
		int max_stat, const Colour& front, const Colour& back) {
	gl_draw_statbar(bbox.translated(-view.x, -view.y), min_stat, max_stat,
			front, back);
}

