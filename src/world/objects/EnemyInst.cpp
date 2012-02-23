#include <cmath>
#include "EnemyInst.h"
#include "PlayerInst.h"
#include "../GameState.h"
#include "../../util/draw_util.h"
#include "../../data/sprite_data.h"

EnemyInst::~EnemyInst() { }

static bool enemy_hit(GameInst* self, GameInst* other){
	return dynamic_cast<EnemyInst*>(other) != NULL;
}

void EnemyInst::init(GameState* gs){
	MonsterController& mc = gs->monster_controller();
	mc.register_enemy(this->id);
}

void EnemyInst::step(GameState* gs){
	GameView& view = gs->window_view();
	PlayerInst* player = (PlayerInst*)gs->player_obj();

	float vx = eb.vx, vy = eb.vy;

	int tx = round(rx+vx/eb.speed*radius);
	int ty = round(ry+vy/eb.speed*radius);

//	if (!gs->tile_radius_test(round(rx+vx),round(ry+vy),RADIUS) &&
//			!gs->object_radius_test(this, NULL, 0, &enemy_hit, tx, ty,1)){
		x = (int)round(rx += vx); //update based on rounding of true float
		y = (int)round(ry += vy);
//	}
}
void EnemyInst::draw(GameState* gs){
	GameView& view = gs->window_view();
	GLImage& img = spr_zombie.img;

	int w = img.width, h = img.height;
	int xx = x-w/2, yy = y-h/2;

	if (!view.within_view(xx,yy, w,h)) return;
	if (!gs->object_visible_test(this)) return;

	int bar_width = 20;
	int hp_width = (20*stat.hp)/stat.max_hp;
	gl_draw_rectangle(view, x-10,y-20,bar_width,5, Colour(255,0,0));
	gl_draw_rectangle(view, x-10,y-20,hp_width,5, Colour(0,255,0));
	//gl_draw_circle(view, x,y,RADIUS);

	image_display(&img, xx-view.x, yy-view.y);
    //gl_draw_rectangle(x,y,50,50, Colour(255,0,0));
	gl_printf(gs->primary_font(), Colour(255,255,255),
			x-10-view.x, y-30-view.y,
			"vx=%g, vy=%g",
			eb.vx, eb.vy);
}
