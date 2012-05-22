#include "PlayerInst.h"

#include "AnimatedInst.h"
#include "EnemyInst.h"
#include "ItemInst.h"
#include "ProjectileInst.h"

#include "../GameState.h"

#include "../../display/display.h"

#include "../../data/sprite_data.h"
#include "../../data/tile_data.h"
#include "../../data/item_data.h"
#include "../../data/weapon_data.h"
#include <typeinfo>

PlayerInst::~PlayerInst() {
}

void PlayerInst::init(GameState* gs) {
	PlayerController& pc = gs->player_controller();
	pc.register_player(this->id, is_local_focus());
}

void PlayerInst::deinit(GameState* gs) {
	PlayerController& pc = gs->player_controller();
	pc.deregister_player(this->id);
	GameInst::deinit(gs);
}

void PlayerInst::step(GameState* gs) {

	if (performed_actions_for_step())
		return;

	gs->level()->steps_left = 1000;
	GameView& view = gs->window_view();

	bool mouse_within = gs->mouse_x() < gs->window_view().width;

	//Stats/effect step
	stats().step();
	effects.step();

	queue_network_actions(gs);

	if (stats().hp <= 0) {
//		if (is_local_focus())
		gs->game_world().reset(0);
//		else
//			gs->remove_instance(this);
		return;
	}

	if (stats().hurt_cooldown > 0)
		canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
	if (--canrestcooldown < 0)
		canrestcooldown = 0;

	isresting = false;
	perform_queued_actions(gs);

	if (!gs->key_down_state(SDLK_x) && is_local_focus())
		view.center_on(last_x, last_y);

	performed_actions_for_step() = true;
}

void PlayerInst::draw(GameState* gs) {
	GameView& view = gs->window_view();
	bool isclient = gs->game_settings().conntype == GameSettings::CLIENT;
	bool isfighterimg = (isclient == is_local_focus());
	GLimage& img = game_sprite_data[get_sprite_by_name(
			isfighterimg ? "fighter" : "wizard")].img;
	bool b = gs->tile_radius_test(x, y, RADIUS);
	//gl_draw_rectangle(view, x-10,y-20,20,5, b ? Colour(255,0,0) : Colour(0,255,0));
	//gl_draw_circle(view, x,y,RADIUS);
	if (stats().hp < stats().max_hp)
		gl_draw_statbar(view, x - 10, y - 20, 20, 5, stats().hp,
				stats().max_hp);
//	if (stats().mp < stats().max_mp)
//					gl_draw_statbar(view, x - 10, y - 25, 20, 5, stats().mp,
//							stats().max_mp, Colour(0,0,255));

	if (effects.get(EFFECT_HASTE)) {
		effect* e = effects.get(EFFECT_HASTE);
		float s = e->t_remaining / 200.0;
		if (s > 1)
			s = 1;
		Colour blue(255 * (1 - s), 255 * (1 - s), 255);
		gl_draw_image(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y, blue);
	} else if (stats().hurt_cooldown > 0) {
		float s = 1 - stats().hurt_alpha();
		Colour red(255, 255 * s, 255 * s);
		gl_draw_image(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y, red);
	} else {
		gl_draw_image(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y);

	}

	if (isresting) {
		GLimage& restimg = game_sprite_data[get_sprite_by_name("resting")].img;
		gl_draw_image(&restimg, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y);
	}

	//for (int i = 0; i < 10; i++)
//	gl_printf(gs->primary_font(), Colour(255,255,255), x-10-view.x, y-30-view.y, "id=%d", this->id);
//	gl_printf(gs->primary_font(), Colour(255,255,255), gs->mouse_x(), gs->mouse_y(),
//			"mx=%d,my=%d\nx=%d,y=%d",
//			gs->mouse_x(), gs->mouse_y(),
//			gs->mouse_x()+view.x, gs->mouse_y()+view.y);

//	gl_printf(gs->primary_font(), Colour(255,255,255), x - view.x, y-25 -view.y, "id=%d", id);
//	gs->monster_controller().paths[0].draw(gs);
}

void PlayerInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*inst = *this;
}

PlayerInst *PlayerInst::clone() const {
	return new PlayerInst(*this);
}
