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
#include "../../data/sprite_data.h"

BulletInst::~BulletInst() {

}

BulletInst::BulletInst(obj_id originator, Attack& attack, int x, int y, int tx, int ty) :
		GameInst(x, y, RADIUS, false), attack(attack), range_left(attack.range), origin_id(originator), rx(x), ry(y) {
	int dx = tx - x, dy = ty - y;
	double abs = sqrt(dx * dx + dy * dy);
	vx = dx * attack.projectile_speed / abs, vy = dy * attack.projectile_speed / abs;
}

static bool enemy_hit(GameInst* self, GameInst* other){
	return dynamic_cast<EnemyInst*>(other) != NULL;
}
static bool player_hit(GameInst* self, GameInst* other){
	return dynamic_cast<PlayerInst*>(other) != NULL;
}

void BulletInst::step(GameState* gs) {
    Pos tile_hit;
    
    int newx = (int) round(rx += vx); //update based on rounding of true float
    int newy = (int) round(ry += vy);
    bool hitsx = gs->tile_radius_test(newx, y, RADIUS, true, -1);
    bool hitsy = gs->tile_radius_test(x, newy, RADIUS, true, -1);
    if(hitsy || hitsx || gs->tile_radius_test(newx, newy, RADIUS, true, -1, &tile_hit)){
        if (hitsx) {
            vx = -vx;
        }
        if (hitsy) {
            vy = -vy;
        }
        if (!hitsy && !hitsx) {
            vx = -vx;
            vy = -vy;
        }
    
    //    return;
    }
	x = (int) round(rx += vx); //update based on rounding of true float
	y = (int) round(ry += vy);

	range_left -= attack.projectile_speed;

	if (range_left <= 0){
		gs->remove_instance(this);
		return;
	}

	GameInst* origin = gs->get_instance(origin_id);
	if (dynamic_cast<PlayerInst*>(origin)){
		GameInst* enemy = NULL;
		gs->object_radius_test(this, &enemy, 1, &enemy_hit);
		if (enemy){
			Stats& s = ((EnemyInst*)enemy)->stats();
			if (s.hurt(attack.damage)) {
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
			s.hurt(attack.damage);
			gs->remove_instance(this);
		}
	}
}

void BulletInst::draw(GameState* gs) {
	GameView& view = gs->window_view();
	GLImage& img = game_sprite_data[attack.projectile_sprite].img;
	int w = img.width, h = img.height;
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;

	image_display(view, &img ,x-TILE_SIZE/2, y-TILE_SIZE/2);
	//gl_draw_circle(view, x, y, RADIUS, (origin_id == gs->local_playerid()) ? Colour(0, 255, 0) : Colour(255, 215, 11));

}
