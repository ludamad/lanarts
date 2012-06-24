/*
 * AnimatedInst.cpp:
 *  Represents an object that displays on the screen for a fixed amount of time before removing itself
 */

#include "AnimatedInst.h"
#include "../GameState.h"
#include "../../data/sprite_data.h"
#include "../../display/display.h"
#include <typeinfo>

AnimatedInst::~AnimatedInst() {}

void AnimatedInst::step(GameState* gs){
	if (timeleft != -1 && --timeleft < 0){
		timeleft = 0;
		gs->remove_instance(this);
	}

}
void AnimatedInst::draw(GameState* gs){
	x = round(rx += vx);
	y = round(ry += vy);
	GameView& view = gs->window_view();
	if (sprite > -1){
		GLimage& img = game_sprite_data[sprite].images[0];

		int w = img.width, h = img.height;
		int xx = x - w / 2, yy = y - h / 2;

		if (!view.within_view(xx, yy, w, h))
			return;
		if (!gs->object_visible_test(this))
			return;

		Colour alphacol(255,255,255, 255*timeleft/animatetime);
		gl_draw_image(img, xx - view.x, yy - view.y, alphacol);
	}
	Colour wd(255 - textcol.r, 255 -textcol.g, 255 - textcol.b);
	if (text.size() > 0){
		Colour alphacol = textcol;
		if (timeleft > -1){
			int fade =100*timeleft/animatetime;
			alphacol = Colour(textcol.r+fade*wd.r/100,
					textcol.g+fade*wd.g/100,
					textcol.b+fade*wd.b/100,
					255 - fade);
		}
		gl_printf(gs->primary_font(), alphacol, x - view.x, y - view.y, "%s", text.c_str());
	}
}


void AnimatedInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(AnimatedInst*)inst = *this;
}

AnimatedInst *AnimatedInst::clone() const {
	return new AnimatedInst(*this);
}


