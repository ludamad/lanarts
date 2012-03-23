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

	image_display(&img, xx - view.x, yy - view.y);
}


void EnemyInst::attack(GameState* gs, GameInst* inst, bool ranged){
	if (stats().has_cooldown()) return;
	PlayerInst* pinst;
	if ( (pinst = dynamic_cast<PlayerInst*>(inst))) {
		GameInst* bullet = new BulletInst(id, stats().bulletspeed, stats().range, x,y,inst->x, inst->y);
		if (ranged){
			gs->add_instance(bullet);
		} else {
			pinst->stats().hp -= 20;
		}
		stats().reset_cooldown();
	}
}
