/*
 * BulletInst.cpp
 *
 *  Created on: 2011-11-11
 *      Author: 100397561
 */

#include "BulletInst.h"
#include "../GameState.h"
#include <cmath>

BulletInst::~BulletInst() {
}

BulletInst::BulletInst(int x, int y, int tx, int ty) :
	GameInst(x, y, RADIUS, false), rx(x), ry(y){
	int dx = tx - x, dy = ty - y;
	double abs = sqrt(dx*dx+dy*dy);
	vx = dx * 7.0 / abs, vy = dy * 7.0 / abs ;
}

void BulletInst::step(GameState* gs){
	x = (int)round(rx += vx);//update based on rounding of true float
	y = (int)round(ry += vy);
	bool b = gs->tile_radius_test(x,y, RADIUS);
	if (b) gs->remove_instance(this);
}
void BulletInst::draw(GameState* gs){
	GameView& view = gs->window_view();
	gl_draw_circle(view, x,y,RADIUS, Colour(0,255,0));
}
