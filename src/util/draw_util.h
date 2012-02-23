#ifndef DRAW_UTIL_H_
#define DRAW_UTIL_H_

#include <SDL.h>
#include <GL/glu.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <string>
#include "font_util.h"
#include "../world/GameView.h"

struct Colour {
    int r, g, b, a;
    Colour(int r, int g, int b, int a = 255) :
        r(r), g(g), b(b), a(a) {}
    operator SDL_Color(){
    	SDL_Color ret = {r,g,b,a};
    	return ret;
    }
};

void gl_draw_circle(float x, float y, float radius,
    const Colour& colour = Colour(0,0,0) );
void gl_draw_rectangle(int x, int y, int w, int h,
    const Colour& colour = Colour(0,0,0) );
inline void gl_draw_circle(const GameView& view, float x, float y, float radius,
    const Colour& colour = Colour(0,0,0) ){
	gl_draw_circle(x-view.x,y-view.y,radius,colour);
}
inline void gl_draw_rectangle(const GameView& view, int x, int y, int w, int h,
    const Colour& colour = Colour(0,0,0)) {
	gl_draw_rectangle(x-view.x,y-view.y,w,h,colour);
}

void gl_draw_rectangle_parts(int x, int y, int w, int h, int sub_parts,
		char* flags, const Colour& colour = Colour(0, 0, 0));

//Will print out text at window coordinates x,y, using the font ft_font.
//The current modelview matrix will also be applied to the text.
void gl_printf(const font_data &ft_font, const Colour& colour, float x, float y, const char *fmt, ...) ;
#endif
