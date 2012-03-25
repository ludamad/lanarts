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
#include "AnimatedInst.h"

BulletInst::~BulletInst() {

}

BulletInst::BulletInst(obj_id originator, Attack& attack, int x, int y, int tx, int ty,
		bool bounce, int hits, GameInst* target) :
		GameInst(x, y, RADIUS, false), attack(attack), range_left(attack.range), origin_id(originator),
		rx(x), ry(y), bounce(bounce), hits(hits), target(target) {
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

static bool target_hit(GameInst* self, GameInst* other){
	return ((BulletInst*)self)->hit_target() == other;
}

void BulletInst::step(GameState* gs) {
    Pos tile_hit;
	int newx = (int) round(rx + vx); //update based on rounding of true float
	int newy = (int) round(ry + vy);
    bool collides = gs->tile_radius_test(newx, newy, RADIUS, true, -1, &tile_hit);
    if (bounce){
		bool hitsx = gs->tile_radius_test(newx, y, RADIUS, true, -1);
		bool hitsy = gs->tile_radius_test(x, newy, RADIUS, true, -1);
		if(hitsy || hitsx || collides){
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
    } else if (collides){
    	gs->remove_instance(this);
    }
	x = (int) round(rx += vx); //update based on rounding of true float
	y = (int) round(ry += vy);

	range_left -= attack.projectile_speed;



	bool hit = false;
	GameInst* colobj = NULL;
	GameInst* origin = gs->get_instance(origin_id);
	if (dynamic_cast<PlayerInst*>(origin)){
		if (target)
			gs->object_radius_test(this, &colobj, 1, &target_hit);
		else
			gs->object_radius_test(this, &colobj, 1, &enemy_hit);
		if (colobj){
			EnemyInst* e = (EnemyInst*)colobj;
			if (e->hurt(gs, attack.damage)) {
				((PlayerInst*)origin)->stats().gain_xp(e->xpworth());
			}
		}
	} else {
		gs->object_radius_test(this, &colobj, 1, &player_hit);
		if (colobj){
			Stats& s = ((PlayerInst*)colobj)->stats();
			s.hurt(attack.damage);
		}
	}
	if (colobj || range_left <= 0){
			hits --;
			if (hits > 0 && colobj){
				MonsterController& mc = gs->monster_controller();
				int mindist = 50000;
				target = NULL;
				for (int i = 0; i < mc.monster_ids().size(); i++){
					obj_id mid = mc.monster_ids()[i];
					GameInst* enemy = gs->get_instance(mid);
					if (enemy && enemy != colobj){

						int dx = enemy->x - x, dy = enemy->y - y;
						double abs = sqrt(dx * dx + dy * dy);
						if (abs < mindist) {
							target = enemy;
							mindist = abs;
							vx = dx * attack.projectile_speed / abs, vy = dy * attack.projectile_speed / abs;
						}
					}
				}
			}
			if (hits == 0 || target == NULL){
				gs->add_instance(new AnimatedInst(x,y,attack.projectile_sprite, 15));
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
