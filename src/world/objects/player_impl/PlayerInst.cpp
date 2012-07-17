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

#include "../../../util/math_util.h"

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

static Pos seen_square_in_area(MTwist& mt, GameTiles& tiles) {
	Pos p, ret;
	do {
		p.x = mt.rand(tiles.tile_width());
		p.y = mt.rand(tiles.tile_height());
	} while (!tiles.is_seen(p.x, p.y) || tiles.is_solid(p.x, p.y));
	return centered_multiple(p, TILE_SIZE);
}
static void spawn_in_lower_level(GameState* gs, PlayerInst* player) {
	int levelid1 = gs->game_world().get_current_level_id();
	int levelid2 = (levelid1 > 0) ? levelid1 - 1 : 0;
	GameLevelState* level2 = gs->game_world().get_level(levelid2);
	Pos sqr = seen_square_in_area(gs->rng(), level2->tiles);

	if (player->is_local_player()) {
		gs->game_chat().add_message("You have respawned!",
				Colour(100, 150, 150));
	} else {
		gs->game_chat().add_message("Your ally has respawned!",
				Colour(100, 150, 150));
	}
	if (levelid1 != levelid2) {
		gs->game_world().level_move(player->id, sqr.x, sqr.y, levelid1,
				levelid2);
	} else {
		player->update_position(sqr.x, sqr.y);
	}
}

void PlayerInst::step(GameState* gs) {

	if (performed_actions_for_step())
		return;

	CombatGameInst::step(gs);
	GameView& view = gs->window_view();

	queue_network_actions(gs);

	//Stats/effect step
	if (cooldowns().is_hurting())
		reset_rest_cooldown();

	if (stats().has_died()) {
		deaths++;
//		if (is_local_focus())
		queued_actions.clear();
//		gs->game_world().reset(0);
		stats().core.heal_fully();
		spawn_in_lower_level(gs, this);
//		else
//			gs->remove_instance(this);
		return;
	}

	is_resting = false;
	perform_queued_actions(gs);

	if (!gs->key_down_state(SDLK_x) && is_local_player())
		view.center_on(last_x, last_y);

	performed_actions_for_step() = true;

	update_position();
}

void PlayerInst::draw(GameState* gs) {
	CombatGameInst::draw(gs);
//	Test io event firing
//	for (int i = 0; i < 5; i++) {
//
//		IOEvent event(IOEvent::SWITCH_TO_SPELL_N, i);
//		if (gs->io_controller().query_event(event)) {
//			gl_printf(gs->primary_font(), Colour(), x - gs->window_view().x,
//					y - gs->window_view().y, "%d", i);
//		}
//	}
}

void PlayerInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(PlayerInst*)inst = *this;
}

PlayerInst *PlayerInst::clone() const {
	return new PlayerInst(*this);
}
