#include <cmath>
#include "EnemyInst.h"
#include "PlayerInst.h"
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
//	GameView& view = gs->window_view();
//	PlayerInst* player = (PlayerInst*) gs->player_obj();

//	int tx = round(rx + vx / eb.speed * radius);
//	int ty = round(ry + vy / eb.speed * radius);

//	if (!gs->tile_radius_test(round(rx+vx),round(ry+vy),RADIUS) &&
//			!gs->object_radius_test(this, NULL, 0, &enemy_hit, tx, ty,1)){
	x = (int) round(rx += eb.vx); //update based on rounding of true float
	y = (int) round(ry += eb.vy);
//	}
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

	if (stat.hp < stat.max_hp)
		gl_draw_hpbar(view, stat, x - 10, y - 20, 20, 5);

	image_display(&img, xx - view.x, yy - view.y);
}


void EnemyInst::attack(GameInst* inst){
	if (stats().has_cooldown()) return;
	PlayerInst* pinst;
	if ( (pinst = dynamic_cast<PlayerInst*>(inst)) ){
		pinst->stats().hp -= 10;
	}
	stats().reset_cooldown();
}
