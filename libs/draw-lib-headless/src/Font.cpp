#include <algorithm>
#include <stdexcept>
#include <vector>

//#include <ft2build.h>
//#include <freetype/freetype.h>
//#include <freetype/ftglyph.h>
//#include <freetype/ftoutln.h>
//#include <freetype/fttrigon.h>

#include <lcommon/math_util.h>
#include <lcommon/geometry.h>

#include <lcommon/perf_timer.h>
#include <lcommon/strformat.h>

#include "DrawOptions.h"

#include "Font.h"
#include "ldraw_assert.h"

#include "GLImage.h"

namespace ldraw {

/******************************************************************
 *  Initialization routines                                       *
 ******************************************************************/
struct font_data {
};

/*Initialize a font of size 'h' from a font file.*/
void init_font(font_data* fd, const char* fname, unsigned int h) {
}

/******************************************************************
 *  Text display routines & helper functions                      *
 ******************************************************************/

static void gl_draw_glyph(const font_data& font, char glyph, const PosF& pos,
		const SizeF& scale) {}

/*Splits up strings, respecting space boundaries & returns maximum width */
static int process_string(const font_data& font, const char* text,
		int max_width, std::vector<int>& line_splits) {
    return 0;
}

//
/* General gl_print function for others to delegate to */
static SizeF gl_print_impl(const DrawOptions& options, const font_data& font,
		PosF p, int maxwidth, bool actually_print, const char* text) {
    return SizeF {0.0f, 0.0f};
}

/******************************************************************
 *  Font class functions                                          *
 ******************************************************************/

void Font::initialize(const char* filename, int height) {
	_font.set(new font_data);
	init_font(_font.get(), filename, height);
}

Font::Font(const BitmapFontDesc& desc) {
	_font.set(new font_data);
}

void Font::draw_wrapped(const DrawOptions& options, const PosF& position,
		int maxwidth, const char* str) const {
	gl_print_impl(options, *_font, position, maxwidth, true, str);
}

void Font::drawf_wrapped(const DrawOptions& options, const PosF& position,
		int maxwidth, const char* fmt, ...) const {
	VARARG_STR_FORMAT(_print_buffer, fmt);
	draw_wrapped(options, position, maxwidth, _print_buffer);
}

int Font::draw(const DrawOptions& options, const PosF& position,
		const char* str) const {
	Colour c = options.draw_colour;
	SizeF size = gl_print_impl(options, *_font, position, -1, true, str);
	return size.w;
}

int Font::drawf(const DrawOptions& options, const PosF& position,
		const char* fmt, ...) const {
	VARARG_STR_FORMAT(_print_buffer, fmt);
	return draw(options, position, _print_buffer);
}

SizeF Font::get_draw_size(const char* str, int maxwidth) const {
	return gl_print_impl(DrawOptions(), *_font, PosF(), maxwidth, false, str);
}

SizeF Font::get_draw_size(const std::string& str, int maxwidth) const {
	return get_draw_size(str.c_str(), maxwidth);
}
int Font::height() const {
	return 0;//_font->h;
}

void Font::clear() {
	_font = smartptr<font_data>();
}

}

