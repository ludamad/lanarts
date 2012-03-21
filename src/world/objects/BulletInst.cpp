/*
 * BulletInst.cpp
 *
 *  Created on: 2011-11-11
 *      Author: 100397561
 */

#include "BulletInst.h"
#include "EnemyInst.h"
#include "PlayerInst.h"
#include "../GameState.h"
#include <cmath>

BulletInst::~BulletInst() {

}

BulletInst::BulletInst(obj_id originator, int speed, int range, int x, int y, int tx, int ty) :
		GameInst(x, y, RADIUS, false), speed(speed), range_left(range), origin_id(originator), rx(x), ry(y) {
	int dx = tx - x, dy = ty - y;
	double abs = sqrt(dx * dx + dy * dy);
	vx = dx * speed / abs, vy = dy * speed / abs;
}

static bool enemy_hit(GameInst* self, GameInst* other){
	return dynamic_cast<EnemyInst*>(other) != NULL;
}
static bool player_hit(GameInst* self, GameInst* other){
	return dynamic_cast<PlayerInst*>(other) != NULL;
}

void BulletInst::step(GameState* gs) {
	x = (int) round(rx += vx); //update based on rounding of true float
	y = (int) round(ry += vy);

	range_left -= speed;

	if (range_left <= 0 || gs->tile_radius_test(x, y, RADIUS)){
		gs->remove_instance(this);
		return;
	}

	GameInst* origin = gs->get_instance(origin_id);
	if (dynamic_cast<PlayerInst*>(origin)){
		GameInst* enemy = NULL;
		gs->object_radius_test(this, &enemy, 1, &enemy_hit);
		if (enemy){
			Stats& s = ((EnemyInst*)enemy)->stats();
			s.hp -= 10;
			if (s.hp <= 0) {
				s.hp = 0;
				gs->remove_instance(enemy);
				((PlayerInst*)origin)->stats().gain_xp(10);
			}
			gs->remove_instance(this);
		}
	} else {
		GameInst* player = NULL;
		gs->object_radius_test(this, &player, 1, &player_hit);
		if (player){
			Stats& s = ((PlayerInst*)player)->stats();
			s.hp -= 10;
			gs->remove_instance(this);
		}
	}
}

void BulletInst::draw(GameState* gs) {
	GameView& view = gs->window_view();
	gl_draw_circle(view, x, y, RADIUS, (origin_id == gs->local_playerid()) ? Colour(0, 255, 0) : Colour(255,0,0));
}
