#include "PlayerInst.h"
#include "BulletInst.h"

#include "../../util/draw_util.h"
#include "../GameState.h"

static const int RADIUS = 10;
PlayerInst::~PlayerInst() {
}
void PlayerInst::step(GameState* gs){
	if (gs->key_press_state(SDLK_UP)){
		if (!gs->tile_radius_test(x, y-4, RADIUS))
			y -= 4;
	}
	if (gs->key_press_state(SDLK_RIGHT)){
		if (!gs->tile_radius_test(x+4, y, RADIUS))
			x += 4;
	}
	if (gs->key_press_state(SDLK_DOWN)){
		if (!gs->tile_radius_test(x, y+4, RADIUS))
		y += 4;
	}
	if (gs->key_press_state(SDLK_LEFT)){
		if (!gs->tile_radius_test(x-4, y, RADIUS))
		x -= 4;
	}

	if (cooldown > 0) cooldown--;
	if (gs->key_press_state(SDLK_f) && cooldown <= 0){
		GameView& view = gs->window_view();
		int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();
		GameInst* bullet = new BulletInst(x,y,rmx, rmy);
		gs->add_instance(bullet);
		cooldown = 10;
	}
	if (gs->mouse_left_click()){
		GameView& view = gs->window_view();
		int nx = gs->mouse_x() + view.x, ny = gs->mouse_y() + view.y;
		if (!gs->tile_radius_test(nx,ny, RADIUS)){
			x = nx, y = ny;
		}
	}
	gs->player_id() = id;
}
void PlayerInst::draw(GameState* gs){
	GameView& view = gs->window_view();
	bool b = gs->tile_radius_test(x,y, RADIUS);
	gl_draw_rectangle(view, x-10,y-20,20,5, b ? Colour(255,0,0) : Colour(0,255,0));
	gl_draw_circle(view, x,y,RADIUS);
	//for (int i = 0; i < 10; i++)
	gl_printf(gs->primary_font(), Colour(255,255,255), x-10-view.x, y-30-view.y, "id=%d", this->id);
	gl_printf(gs->primary_font(), Colour(255,255,255), gs->mouse_x(), gs->mouse_y(),
			"mx=%d,my=%d\nx=%d,y=%d",
			gs->mouse_x(), gs->mouse_y(),
			gs->mouse_x()+view.x, gs->mouse_y()+view.y);

	view.center_on(x,y);
}
