/*
 * display.h:
 *  Image drawing and other drawing related utility functions
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <SDL.h>
#undef GL_GLEXT_VERSION
#include <SDL_opengl.h>
#include <GL/glu.h>
#include "../util/game_basic_structs.h"

struct GLimage;
struct font_data;
struct GameView;
struct SpriteEntry;

void gl_set_drawing_area(int x, int y, int w, int h);
void init_sdl_gl(bool fullscreen, int w, int h);

void update_display();
void SDL_GL_diagnostics();

void gl_image_from_bytes(GLimage& img, int w, int h, char* data, int type =
		GL_BGRA);

void gl_draw_image(GLimage& img, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void gl_draw_image(const GameView& view, GLimage& img, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void gl_draw_sprite(sprite_id sprite, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void gl_draw_sprite(sprite_id sprite, const GameView& view, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void gl_draw_sprite_entry(const GameView& view, SpriteEntry& entry, int x,
		int y, float dx, float dy, int steps,
		const Colour& c = Colour(255, 255, 255));

void gl_draw_circle(float x, float y, float radius, const Colour& colour =
		Colour(0, 0, 0), bool outline = false);
void gl_draw_rectangle(int x, int y, int w, int h, const Colour& colour =
		Colour(0, 0, 0));
void gl_draw_circle(const GameView& view, float x, float y, float radius,
		const Colour& colour = Colour(0, 0, 0), bool outline = false);
void gl_draw_rectangle(const GameView& view, int x, int y, int w, int h,
		const Colour& colour = Colour(0, 0, 0));

void gl_draw_rectangle_outline(int x, int y, int w, int h, const Colour& clr =
		Colour(255, 255, 255), int linewidth = 1);

inline void gl_draw_rectangle_outline(const BBox& bbox,
		const Colour& clr = Colour(255, 255, 255), int linewidth = 1) {
	gl_draw_rectangle_outline(bbox.x1, bbox.y1, bbox.width(), bbox.height(),
			clr, linewidth);
}

void gl_draw_line(int x1, int y1, int x2, int y2,
		const Colour& clr = Colour(255, 255, 255), int linewidth = 1);

void gl_draw_statbar(const BBox& bbox, int min_stat, int max_stat,
		const Colour& front = Colour(0, 255, 0),
		const Colour& back = Colour(255, 0, 0));
void gl_draw_statbar(const GameView& view, const BBox& bbox, int min_stat,
		int max_stat, const Colour& front = Colour(0, 255, 0),
		const Colour& back = Colour(255, 0, 0));

void gl_draw_rectangle_parts(int x, int y, int w, int h, int sub_parts,
		char* flags, const Colour& colour = Colour(0, 0, 0));

//Will print out text at window coordinates x,y, using the font ft_font.
//The current modelview matrix will also be applied to the text.
Pos gl_printf(const font_data &ft_font, const Colour& colour, float x, float y,
		const char *fmt, ...);
Pos gl_printf_bounded(const font_data& font, const Colour& colour, float x,
		float y, int max_width, const char *fmt, ...);
/* printf-like function that draws to the screen, returns width of formatted string*/
Pos gl_printf_centered(const font_data& font, const Colour& colour, float x,
		float y, const char *fmt, ...);

#endif /* DISPLAY_H_ */
