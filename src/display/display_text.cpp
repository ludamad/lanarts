/*
 * display_text.cpp:
 *  Implements text display routines & helper functions
 */

#include <vector>

#undef GL_GLEXT_VERSION
#include <SDL_opengl.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "display.h"
#include "font.h"
#include "GLImage.h"

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
		if (c == '\n' || (overmax && !isspace(c))) {
			line_splits.push_back(last_space + 1);
			largest_width = std::max(width, largest_width);
			width = width_since_space;
		}
		ind++;
	}
	line_splits.push_back(ind);

	largest_width = std::max(width, largest_width);

	return largest_width;
}

#ifndef __GNUC__
#define vsnprintf(text, len, fmt, ap) vsprintf(text, fmt, ap)
#endif

//
/* General gl_print function for others to delegate to */
static Dim gl_print_impl(const font_data& font, const Colour& colour, Pos p,
		int max_width, bool center_x, bool center_y, bool actually_print,
		const char* fmt, va_list ap) {
	char text[512];
	vsnprintf(text, 512, fmt, ap);
	va_end(ap);

	Dim offset(0, 0);

	std::vector<int> line_splits;
	int measured_width = process_string(font, text, max_width, line_splits);

	if (center_x) {
		p.x -= measured_width / 2;
	}
	if (center_y) {
		p.y -= font.h * line_splits.size() / 2;
	}

	for (int linenum = 0, i = 0; linenum < line_splits.size(); linenum++) {
		int len = 0;
		int eol = line_splits[linenum];

		offset.h += font.h;

		for (; i < eol; i++) {
			unsigned char chr = text[i];
			if (chr == '\n') {
				continue; //skip newline char
			}
			char_data &cdata = *font.data[chr];
			len += cdata.advance;
			if (actually_print) {
				gl_draw_image(cdata.img,
						p.x + len - (cdata.advance - cdata.left),
						p.y + offset.h - cdata.move_up, colour);
			}
		}
		offset.w = std::max(len, offset.w);
		offset.h += 1;
	}
	return offset;
}
/* printf-like function that draws to the screen, returns dimensions of formatted string*/
Dim gl_printf(const font_data& font, const Colour& colour, float x, float y,
		const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), -1, false, false, true, fmt,
			ap);
}

/* printf-like function that draws to the screen, returns width of formatted string*/
Dim gl_printf_bounded(const font_data& font, const Colour& colour, float x,
		float y, int max_width, bool center_y, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), max_width, false, center_y,
			true, fmt, ap);
}
/* printf-like function that draws to the screen, returns width of formatted string*/
Dim gl_printf_y_centered_bounded(const font_data& font, const Colour& colour,
		float x, float y, int max_width, bool center_y, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), max_width, false, true, true,
			fmt, ap);
}

Dim gl_text_dimensions(const font_data& font, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, Colour(), Pos(), -1, false, false, false, fmt,
			ap);
}

/* printf-like function that draws to the screen, returns width of formatted string*/
Dim gl_printf_centered(const font_data& font, const Colour& colour, float x,
		float y, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), -1, true, true, true, fmt, ap);
}

/* printf-like function that draws to the screen, returns width of formatted string*/
Dim gl_printf_y_centered(const font_data& font, const Colour& colour, float x,
		float y, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), -1, false, true, true, fmt,
			ap);
}

/* printf-like function that draws to the screen, returns width of formatted string*/
Dim gl_printf_x_centered(const font_data& font, const Colour& colour, float x,
		float y, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	return gl_print_impl(font, colour, Pos(x, y), -1, true, false, true, fmt,
			ap);
}
