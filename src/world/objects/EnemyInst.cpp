#include <cmath>
#include "EnemyInst.h"
#include "PlayerInst.h"
#include "BulletInst.h"
#include "../GameState.h"
#include "../../util/draw_util.h"
#include "../../data/sprite_data.h"

EnemyInst::~EnemyInst() {
}

static bool enemy_hit(GameInst* self, GameInst* other) {
	return dynamic_cast<EnemyInst*>(other) != NULL;
}

void EnemyInst::init(GameState* gs) {
	MonsterController& mc = gs->monster_controller();
	mc.register_enemy(this->id);
	stats().hp += stats().hp*gs->branch_level()/10.0;
	stats().max_hp += stats().max_hp*gs->branch_level()/10.0;
	stats().mp += stats().mp*gs->branch_level()/10.0;
	stats().max_mp += stats().max_mp*gs->branch_level()/10.0;
	stats().ranged.cooldown /= 1.0 + gs->branch_level()/10.0;
	stats().ranged.damage *= 1.0 + gs->branch_level()/10.0;
	stats().ranged.projectile_speed *= 1.0 + gs->branch_level()/10.0;
	stats().melee.cooldown /= 1.0 + gs->branch_level()/10.0;
	stats().melee.damage *= 1.0 + gs->branch_level()/10.0;
	stats().max_mp += stats().max_mp*gs->branch_level()/10.0;
	double speedfactor = 1+stats().max_mp*gs->branch_level()/10.0;
	if (stats().movespeed < 3)
		eb.speed = std::min(stats().movespeed*speedfactor,3.0);
}

void EnemyInst::step(GameState* gs) {
	bool firstcol = true;
	GameInst* collided = NULL;
	gs->object_radius_test(this, &collided, 1, &enemy_hit);
	if (!collided){
		gs->object_radius_test(this, &collided, 1, &enemy_hit, x + eb.vx, y + eb.vy);
		firstcol = false;
	}
	if (collided ){
		float dx = collided->x - x, dy = collided->y - y;
		float mag = sqrt(dx*dx + dy*dy);
		/*if (mag == 0) mag = 1;
			eb.vx = -dy*eb.speed/mag;
			eb.vy = -dx*eb.speed/mag;
		if (!firstcol && gs->object_radius_test(this, NULL, 0, &enemy_hit, x + eb.vx, y +eb.vy)){
			eb.vx = 0;
			eb.vy = 0;
		}*/
		if (gs->tile_radius_test(x+eb.vx, y+eb.vy, radius)){
			eb.vx = -eb.vx;
			eb.vy = -eb.vy;
		}
	}
	if (stats().hurt_cooldown > 0)
		eb.vx /=2, eb.vy /=2 ;
	x = (int) round(rx += eb.vx); //update based on rounding of true float
	y = (int) round(ry += eb.vy);

//	}
	stats().step();
}
void EnemyInst::draw(GameState* gs) {
	GameView& view = gs->window_view();
	GLImage& img = game_sprite_data[type->sprite_number].img;

	int w = img.width, h = img.height;
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;

	if (stats().hp < stats().max_hp)
		gl_draw_statbar(view, x - 10, y - 20, 20, 5, stats().hp, stats().max_hp);

	if (stats().hurt_cooldown > 0){
		float s = 1 - stats().hurt_alpha();
		Colour red(255,s,s);
		image_display(&img, xx - view.x, yy - view.y, red);
	}else{
		image_display(&img, xx - view.x, yy - view.y);
	}
}


void EnemyInst::attack(GameState* gs, GameInst* inst, bool ranged){
	if (stats().has_cooldown()) return;
	PlayerInst* pinst;
	if ( (pinst = dynamic_cast<PlayerInst*>(inst))) {
		if (ranged){
			Attack& ranged = stats().ranged;
			GameInst* bullet = new BulletInst(id, ranged, x,y,inst->x, inst->y);
			gs->add_instance(bullet);
			stats().reset_ranged_cooldown();
		} else {
			pinst->stats().hurt(stats().melee.damage);
			stats().reset_melee_cooldown();
		}
	}
}
