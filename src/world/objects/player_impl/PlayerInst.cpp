/*
 * PlayerInst.cpp
 *  Implements the standard GameInst actions such as step, draw, etc for PlayerInst
 */

#include <typeinfo>

#include "../../utility_objects/AnimatedInst.h"

#include "../../GameState.h"
#include "../../../data/sprite_data.h"
#include "../../../data/tile_data.h"
#include "../../../data/item_data.h"
#include "../../../data/weapon_data.h"

#include "../../../display/display.h"

#include "../EnemyInst.h"
#include "../ItemInst.h"
#include "../PlayerInst.h"
#include "../ProjectileInst.h"

PlayerInst::~PlayerInst() {
}

bool PlayerInst::within_field_of_view(const Pos& pos) {
	return fieldofview.within_fov(pos.x / TILE_SIZE, pos.y / TILE_SIZE);
}

void PlayerInst::die(GameState* gs) {
	//Let step event handle death
}

void PlayerInst::init(GameState* gs) {
	CombatGameInst::init(gs);
	PlayerController& pc = gs->player_controller();
	pc.register_player(this->id, is_local_player());
}

void PlayerInst::deinit(GameState* gs) {
	PlayerController& pc = gs->player_controller();
	pc.deregister_player(this->id);
	CombatGameInst::deinit(gs);
}

void PlayerInst::step(GameState* gs) {

	if (performed_actions_for_step())
		return;

	CombatGameInst::step(gs);

	gs->get_level()->steps_left = 1000;
	GameView& view = gs->window_view();

	queue_network_actions(gs);

	//Stats/effect step
	if (cooldowns().is_hurting())
		reset_rest_cooldown();

	if (stats().has_died()) {
//		if (is_local_focus())
		queued_actions.clear();
		gs->game_world().reset(0);
//		else
//			gs->remove_instance(this);
		return;
	}

	isresting = false;
	perform_queued_actions(gs);

	if (!gs->key_down_state(SDLK_x) && is_local_player())
		view.center_on(last_x, last_y);

	performed_actions_for_step() = true;
}

void PlayerInst::draw(GameState* gs) {
	CombatGameInst::draw(gs);
}

void PlayerInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(PlayerInst*)inst = *this;
}

PlayerInst *PlayerInst::clone() const {
	return new PlayerInst(*this);
}
