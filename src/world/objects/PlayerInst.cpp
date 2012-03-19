#include "PlayerInst.h"
#include "BulletInst.h"

#include "../../util/draw_util.h"
#include "../GameState.h"
#include "../../data/sprite_data.h"
#include "../../data/tile_data.h"
#include "../../display/display.h"

PlayerInst::~PlayerInst() {
}

void PlayerInst::init(GameState* gs){
	PlayerController& pc = gs->player_controller();
	pc.register_player(this->id);
}

void PlayerInst::step(GameState* gs){
	GameView& view = gs->window_view();


	if (gs->key_press_state(SDLK_UP) || gs->key_press_state(SDLK_w)){
		if (!gs->tile_radius_test(x, y-4, RADIUS))
			y -= 4;
	}
	if (gs->key_press_state(SDLK_RIGHT) || gs->key_press_state(SDLK_d)){
		if (!gs->tile_radius_test(x+4, y, RADIUS))
			x += 4;
	}
	if (gs->key_press_state(SDLK_DOWN) || gs->key_press_state(SDLK_s)){
		if (!gs->tile_radius_test(x, y+4, RADIUS))
		y += 4;
	}
	if (gs->key_press_state(SDLK_LEFT) || gs->key_press_state(SDLK_a)){
		if (!gs->tile_radius_test(x-4, y, RADIUS))
		x -= 4;
	}
	if (gs->key_press_state(SDLK_c)){
		if (gs->tile_radius_test(x, y, RADIUS, false, TILE_STAIR_DOWN)){
			gs->set_generate_flag();
		}
	}

	base_stats.step();

	if ((gs->key_press_state(SDLK_f) || gs->mouse_left_click()) && !base_stats.has_cooldown()){
		int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();
		GameInst* bullet = new BulletInst(x,y,rmx, rmy);
		gs->add_instance(bullet);
		base_stats.reset_cooldown();
	}
	if (gs->mouse_right_click()){
		int nx = gs->mouse_x() + view.x, ny = gs->mouse_y() + view.y;
		view.center_on(nx, ny);
	} else
		view.center_on(last_x,last_y);
	gs->player_id() = id;
}

void PlayerInst::draw(GameState* gs){
	GameView& view = gs->window_view();
	GLImage& img = spr_player.img;
	bool b = gs->tile_radius_test(x,y, RADIUS);
	gl_draw_rectangle(view, x-10,y-20,20,5, b ? Colour(255,0,0) : Colour(0,255,0));
	//gl_draw_circle(view, x,y,RADIUS);

	image_display(&img, x-img.width/2-view.x, y-img.height/2-view.y);
	//for (int i = 0; i < 10; i++)
	gl_printf(gs->primary_font(), Colour(255,255,255), x-10-view.x, y-30-view.y, "id=%d", this->id);
	gl_printf(gs->primary_font(), Colour(255,255,255), gs->mouse_x(), gs->mouse_y(),
			"mx=%d,my=%d\nx=%d,y=%d",
			gs->mouse_x(), gs->mouse_y(),
			gs->mouse_x()+view.x, gs->mouse_y()+view.y);

//	gs->monster_controller().paths[0].draw(gs);
}

