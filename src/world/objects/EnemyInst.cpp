#include <cmath>
#include "EnemyInst.h"
#include "../../util/draw_util.h"
#include "../GameState.h"

const int RADIUS = 10;

EnemyInst::~EnemyInst() { }

void EnemyInst::step(GameState* gs){
	int nx = x+vx,ny = y+vy;
	if (gs->tile_radius_test(nx,y,RADIUS)){
		vx = -vx;
	} else if (gs->tile_radius_test(x,ny,RADIUS)){
		vy = -vy;
	} else if (gs->tile_radius_test(nx,ny,RADIUS)){
		vx = -vx, vy = -vy;
	} else {
		x = nx, y = ny;
	}
	steps++;
}
void EnemyInst::draw(GameState* gs){
	GameView& view = gs->window_view();
	gl_draw_circle(view, x,y,RADIUS, Colour(255,255,255));
	gl_printf(gs->primary_font(), Colour(255,255,255),
			x-10-view.x, y-30-view.y, "id=%d", this->id);
    //gl_draw_rectangle(x,y,50,50, Colour(255,0,0));
}
