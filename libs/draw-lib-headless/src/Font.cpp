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
	//FT_Library library;
	//fd->h = h;

	//if (FT_Init_FreeType(&library)) {
	//	throw std::runtime_error( format("Font initialization for '%s' failed!", fname) );
	//}

	//FT_Face face;

	////This is where we load in the font information from the file.
	////Of all the places where the code might die, this is the most likely,
	////as FT_New_Face will die if the font file does not exist or is somehow broken.
	//if (FT_New_Face(library, fname, 0, &face)) {
	//	throw std::runtime_error(
	//			 format("Font '%s' does not exist, or is an invalid font file!", fname));
	//}

	////For some twisted reason, Freetype measures font size
	////in terms of 1/64ths of pixels.  Thus, to make a font
	////h pixels high, we need to request a size of h*64.
	//FT_Set_Char_Size(face, h * 64, h * 64, 96, 96);

	//int maxw = 0;
	//int maxh = 0;
	//for (int i = 0; i < 128; i++) {
	//	char_data& data = fd->data[i];
	//	data.init(i, face);
	//	data.imgoffset = maxw;
	//	maxw += data.w;
	//	maxh = std::max(maxh, data.h);
	//}


	//float ptw = power_of_two_round(maxw), pth = power_of_two_round(maxh);
	//fd->font_img.initialize(Size(ptw, pth));
	//for (int i = 0; i < 128; i++) {
	//	char_data& data = fd->data[i];
	//	int offset = data.imgoffset;
	//	data.tx1 = offset / ptw;
	//	data.tx2 = (offset + data.w) / ptw;
	//	data.ty1 = 0 / pth;
	//	data.ty2 = (data.h) / pth;
	//}

	//We don't need the face information now that the display
	//lists have been created, so we free the assosiated resources.
	//FT_Done_Face(face);

	//FT_Done_FreeType(library);
}

/******************************************************************
 *  Text display routines & helper functions                      *
 ******************************************************************/

static void gl_draw_glyph(const font_data& font, char glyph, const PosF& pos,
		const SizeF& scale) {}

/*Splits up strings, respecting space boundaries & returns maximum width */
static int process_string(const font_data& font, const char* text,
		int max_width, std::vector<int>& line_splits) {
//	int last_space = 0, ind = 0;
//	int largest_width = 0;
//	int width = 0, width_since_space = 0;
//	unsigned char c;
//
//	while ((c = text[ind]) != '\0') {
//		const char_data& cdata = font.data[c];
//		width += cdata.advance;
//		width_since_space += cdata.advance;
//
//		if (isspace(c)) {
//			last_space = ind;
//			width_since_space = 0;
//		}
//		bool overmax = max_width != -1 && width > max_width;
//		if (c == '\n' || (overmax && !isspace(c))) {
//			line_splits.push_back(last_space + 1);
//			largest_width = std::max(width, largest_width);
//			width = width_since_space;
//		}
//		ind++;
//	}
//	line_splits.push_back(ind);
//
//	largest_width = std::max(width, largest_width);
//
//	return largest_width;
    return 0;
}

//
/* General gl_print function for others to delegate to */
static SizeF gl_print_impl(const DrawOptions& options, const font_data& font,
		PosF p, int maxwidth, bool actually_print, const char* text) {
	//perf_timer_begin(FUNCNAME);

	//LDRAW_ASSERT(options.draw_region == BBoxF());
	//LDRAW_ASSERT(options.draw_angle == 0.0f);
	//LDRAW_ASSERT(options.draw_frame == 0.0f);
	//LDRAW_ASSERT(options.draw_scale == SizeF(1.0f, 1.0f));

	//std::vector<int> line_splits;
	//int measured_width = process_string(font, text, maxwidth, line_splits);
	//int height = font.h * line_splits.size();

	//p = adjusted_for_origin(p, SizeF(measured_width, height),
	//		options.draw_origin);

	//const Colour& c = options.draw_colour;

	//Size size(0, 0);
	//for (int linenum = 0, i = 0; linenum < line_splits.size(); linenum++) {
	//	int len = 0;
	//	int eol = line_splits[linenum];

	//	size.h += font.h;

	//	for (; i < eol && text[i]; i++) {
	//		unsigned char chr = text[i];
	//		if (chr == '\n') {
	//			continue; //skip newline char
	//		}
	//		const char_data& cdata = font.data[chr];
	//		len += cdata.advance;
	//		if (actually_print) {
	//			Pos drawpos(p.x + len - (cdata.advance - cdata.left),
	//					p.y + size.h - cdata.move_up);
	//			gl_draw_glyph(font, chr, drawpos, options.draw_scale);
	//		}
	//	}
	//	size.w = std::max(len, size.w);
	//	size.h += 1;
	//}
	//perf_timer_end(FUNCNAME);
	//return size;
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

