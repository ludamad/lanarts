/*
 * BulletInst.cpp
 *
 *  Created on: 2011-11-11
 *      Author: 100397561
 */

#include "BulletInst.h"
#include "EnemyInst.h"
#include "../GameState.h"
#include <cmath>

BulletInst::~BulletInst() {

}

BulletInst::BulletInst(int x, int y, int tx, int ty) :
		GameInst(x, y, RADIUS, false), rx(x), ry(y) {
	int dx = tx - x, dy = ty - y;
	double abs = sqrt(dx * dx + dy * dy);
	vx = dx * 7.0 / abs, vy = dy * 7.0 / abs;
}

void BulletInst::step(GameState* gs) {
	x = (int) round(rx += vx); //update based on rounding of true float
	y = (int) round(ry += vy);
	bool b = gs->tile_radius_test(x, y, RADIUS);
	if (b)
		gs->remove_instance(this);
}

static bool enemy_hit(GameInst* self, GameInst* other){
	return dynamic_cast<EnemyInst*>(other) != NULL;
}
void BulletInst::draw(GameState* gs) {
	GameView& view = gs->window_view();
	GameInst* enemy = NULL;
	gs->object_radius_test(this, &enemy, 1, &enemy_hit);
	if (enemy){
		Stats& s = ((EnemyInst*)enemy)->stats();
		s.hp -= 10;
		if (s.hp <= 0) {
			s.hp = 0;
			gs->remove_instance(enemy);
		}
		gs->remove_instance(this);
	}
	gl_draw_circle(view, x, y, RADIUS, Colour(0, 255, 0));
}
