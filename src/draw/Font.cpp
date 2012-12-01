#include <algorithm>
#include <stdexcept>
#include <vector>

#include <GL/glu.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include <common/math.h>
#include <common/geometry.h>

#include <common/perf_timer.h>
#include <common/strformat.h>

#include <draw/DrawOptions.h>

#include "Font.h"
#include "ldraw_assert.h"

#include "opengl/GLImage.h"

namespace ldraw {

/******************************************************************
 *  Initialization routines                                       *
 ******************************************************************/

/*Stores everything needed to render a character glyph in opengl */
struct char_data {
	int imgoffset;
	int w, h;
	int advance;
	int left;
	int move_up;
	unsigned char* data;
	float tx1, tx2, ty1, ty2;

	char_data() {
		memset(this, 0, sizeof(char_data));
	}
	void init(char ch, FT_Face face);
	~char_data();
};

/*Holds all of the information related to any freetype font that we want to create.*/
struct font_data {
	GLImage font_img;
	char_data data[128];
	float h; //Height of the font
	font_data() {
		h = 0;
	}
};

void char_data::init(char ch, FT_Face face) {
	// Load The Glyph For Our Character.
	if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT))
		throw std::runtime_error("FT_Load_Glyph failed");

	// Move The Face's Glyph Into A Glyph Object.
	FT_Glyph glyph;
	if (FT_Get_Glyph(face->glyph, &glyph))
		throw std::runtime_error("FT_Get_Glyph failed");


	// Convert The Glyph To A Bitmap.
	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

	FT_Bitmap& bitmap = bitmap_glyph->bitmap;

	advance = face->glyph->advance.x >> 6;
	left = bitmap_glyph->left;
	w = bitmap.width;
	h = bitmap.rows;
	move_up = bitmap_glyph->top; //-bitmap.rows;
//
	data = new unsigned char[4 * w * h];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			const int my = y; //h-1-y;
			data[4 * (x + w * my)] = 255;
			data[4 * (x + w * my) + 1] = 255;
			data[4 * (x + w * my) + 2] = 255;
			data[4 * (x + w * my) + 3] = bitmap.buffer[x + w * y];
		}
	}
	FT_Done_Glyph(glyph);
}

char_data::~char_data() {
	delete[] data;
}

/*Initialize a font of size 'h' from a font file.*/
void init_font(font_data* fd, const char* fname, unsigned int h) {
	FT_Library library;
	fd->h = h;

	if (FT_Init_FreeType(&library)) {
		throw std::runtime_error(std::string("Font initialization for '") + fname + "' failed!");
	}

	FT_Face face;

	//This is where we load in the font information from the file.
	//Of all the places where the code might die, this is the most likely,
	//as FT_New_Face will die if the font file does not exist or is somehow broken.
	if (FT_New_Face(library, fname, 0, &face))
		throw std::runtime_error(
				"FT_New_Face failed (there is probably a problem with your font file)");

	//For some twisted reason, Freetype measures font size
	//in terms of 1/64ths of pixels.  Thus, to make a font
	//h pixels high, we need to request a size of h*64.
	FT_Set_Char_Size(face, h * 64, h * 64, 96, 96);

	int maxw = 0;
	int maxh = 0;
	for (int i = 0; i < 128; i++) {
		char_data& data = fd->data[i];
		data.init(i, face);
		data.imgoffset = maxw;
		maxw += data.w;
		maxh = std::max(maxh, data.h);
	}


	float ptw = power_of_two(maxw), pth = power_of_two(maxh);
	fd->font_img.initialize(Dim(ptw, pth));
	for (int i = 0; i < 128; i++) {
		char_data& data = fd->data[i];
		int offset = data.imgoffset;
		data.tx1 = offset / ptw;
		data.tx2 = (offset + data.w) / ptw;
		data.ty1 = 0 / pth;
		data.ty2 = (data.h) / pth;
		fd->font_img.subimage_from_bytes(
				BBox(offset, 0, offset + data.w, data.h), (char*)data.data,
				GL_BGRA);
	}

	//We don't need the face information now that the display
	//lists have been created, so we free the assosiated resources.
	FT_Done_Face(face);

	FT_Done_FreeType(library);

}

/******************************************************************
 *  Text display routines & helper functions                      *
 ******************************************************************/

static void gl_draw_glyph(const font_data& font, char glyph, const Posf& pos,
		const DimF& scale) {
	const GLImage& img = font.font_img;
	const char_data& data = font.data[glyph];
	if (data.w == 0 || data.h == 0) {
		return;
	}
	float x2 = pos.x + data.w * scale.w, y2 = pos.y + data.h * scale.h;

	//Draw our four points, clockwise.
	glTexCoord2f(data.tx1, data.ty1);
	glVertex2i(pos.x, pos.y);

	glTexCoord2f(data.tx2, data.ty1);
	glVertex2i(x2, pos.y);

	glTexCoord2f(data.tx2, data.ty2);
	glVertex2i(x2, y2);

	glTexCoord2f(data.tx1, data.ty2);
	glVertex2i(pos.x, y2);
}

/*Splits up strings, respecting space boundaries & returns maximum width */
static int process_string(const font_data& font, const char* text,
		int max_width, std::vector<int>& line_splits) {
	int last_space = 0, ind = 0;
	int largest_width = 0;
	int width = 0, width_since_space = 0;
	unsigned char c;

	while ((c = text[ind]) != '\0') {
		const char_data& cdata = font.data[c];
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

//
/* General gl_print function for others to delegate to */
static DimF gl_print_impl(const DrawOptions& options, const font_data& font,
		Posf p, int maxwidth, bool actually_print, const char* text) {
	perf_timer_begin(FUNCNAME);

	LDRAW_ASSERT(options.draw_region == BBoxF());
	LDRAW_ASSERT(options.draw_angle == 0.0f);
	LDRAW_ASSERT(options.draw_frame == 0.0f);
	LDRAW_ASSERT(options.draw_scale == DimF(1.0f, 1.0f));

	std::vector<int> line_splits;
	int measured_width = process_string(font, text, maxwidth, line_splits);
	int height = font.h * line_splits.size();

	p = adjusted_for_origin(p, DimF(measured_width, height),
			options.draw_origin);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, font.font_img.texture);

	const Colour& c = options.draw_colour;
	glColor4ub(c.r, c.g, c.b, c.a);
	glBegin(GL_QUADS);

	Dim size(0, 0);
	for (int linenum = 0, i = 0; linenum < line_splits.size(); linenum++) {
		int len = 0;
		int eol = line_splits[linenum];

		size.h += font.h;

		for (; i < eol; i++) {
			unsigned char chr = text[i];
			if (chr == '\n') {
				continue; //skip newline char
			}
			const char_data& cdata = font.data[chr];
			len += cdata.advance;
			if (actually_print) {
				Pos drawpos(p.x + len - (cdata.advance - cdata.left),
						p.y + size.h - cdata.move_up);
				gl_draw_glyph(font, chr, drawpos, options.draw_scale);
			}
		}
		size.w = std::max(len, size.w);
		size.h += 1;
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	perf_timer_end(FUNCNAME);
	return size;
}

/******************************************************************
 *  Font class functions                                          *
 ******************************************************************/

void Font::initialize(const char* filename, int height) {
	_font = new font_data;
	init_font(_font.get(), filename, height);
}

void Font::draw_wrapped(const DrawOptions& options, const Posf& position,
		int maxwidth, const char* str) const {
	gl_print_impl(options, *_font, position, maxwidth, true, str);
}

void Font::drawf_wrapped(const DrawOptions& options, const Posf& position,
		int maxwidth, const char* fmt, ...) const {
	VARARG_STR_FORMAT(_print_buffer, fmt);
	draw_wrapped(options, position, maxwidth, _print_buffer);
}

int Font::draw(const DrawOptions& options, const Posf& position,
		const char* str) const {
	Colour c = options.draw_colour;
	DimF size = gl_print_impl(options, *_font, position, -1, true, str);
	return size.w;
}

int Font::drawf(const DrawOptions& options, const Posf& position,
		const char* fmt, ...) const {
	VARARG_STR_FORMAT(_print_buffer, fmt);
	return draw(options, position, _print_buffer);
}

DimF Font::get_draw_size(const char* str, int maxwidth) const {
	return gl_print_impl(DrawOptions(), *_font, Posf(), maxwidth, false, str);
}

DimF Font::get_draw_size(const std::string& str, int maxwidth) const {
	return get_draw_size(str.c_str(), maxwidth);
}
int Font::height() const {
	return _font->h;
}

}

