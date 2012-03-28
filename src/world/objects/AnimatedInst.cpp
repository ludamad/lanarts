/*
 * AnimatedInst.cpp
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */

#include "AnimatedInst.h"
#include "../GameState.h"
#include "../../data/sprite_data.h"
#include "../../util/draw_util.h"

AnimatedInst::~AnimatedInst() {}

void AnimatedInst::step(GameState* gs){
	if (--timeleft < 0){
		timeleft = 0;
		gs->remove_instance(this);
	}

}
void AnimatedInst::draw(GameState* gs){
	x = round(rx += vx);
	y = round(ry += vy);
	GameView& view = gs->window_view();
	if (sprite > -1){
		GLImage& img = game_sprite_data[sprite].img;

		int w = img.width, h = img.height;
		int xx = x - w / 2, yy = y - h / 2;

		if (!view.within_view(xx, yy, w, h))
			return;
		if (!gs->object_visible_test(this))
			return;

		Colour alphacol(255,255,255, 255*timeleft/animatetime);
		image_display(&img, xx - view.x, yy - view.y, alphacol);
	}
	if (text.size() > 0){
		int fade =100*timeleft*2/animatetime;
		Colour alphacol(255,255-fade,255-fade);
		gl_printf(gs->primary_font(), alphacol, x - view.x, y - view.y, "%s", text.c_str());
	}
}
