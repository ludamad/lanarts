#include "font.h"

#include <SDL.h>
#include <GL/glu.h>
#undef GL_GLEXT_VERSION

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include <stdexcept>
#include "display.h"

#include "../util/math_util.h"

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

	data = new unsigned char[4 * w * h];
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) {
			const int my = y; //h-1-y;
			data[4 * (x + w * my)] = 255;
			data[4 * (x + w * my) + 1] = 255;
			data[4 * (x + w * my) + 2] = 255;
			data[4 * (x + w * my) + 3] = bitmap.buffer[x + w * y];
		}
}

char_data::~char_data() {
	delete[] data;
}

void init_font(font_data* fd, const char * fname, unsigned int h) {
	fd->h = h;

	FT_Library library;

	if (FT_Init_FreeType(&library))
		throw std::runtime_error("FT_Init_FreeType failed");

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
	//(h << 6 is just a prettier way of writting h*64)
	FT_Set_Char_Size(face, h << 6, h << 6, 96, 96);

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
	gl_init_image(fd->font_img, ptw, pth);
	for (int i = 0; i < 128; i++) {
		char_data& data = fd->data[i];
		int offset = data.imgoffset;
		data.tx1 = offset / ptw;
		data.tx2 = (offset + data.w) / ptw;
		data.ty1 = 0 / pth;
		data.ty2 = (data.h) / pth;
		gl_subimage_from_bytes(fd->font_img,
				BBox(offset, 0, offset + data.w, data.h), (char*)data.data,
				GL_BGRA);
	}

	//We don't need the face information now that the display
	//lists have been created, so we free the assosiated resources.
	FT_Done_Face(face);

	FT_Done_FreeType(library);

}

void release_font(font_data* fd) {
//	for(int i=0;i<128;i++) delete fd->data[i];
}
