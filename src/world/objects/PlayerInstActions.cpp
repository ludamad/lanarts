/*
 * PlayerInstActions.cpp
 *
 *  Created on: Mar 28, 2012
 *      Author: 100397561
 */

#include "PlayerInst.h"
#include "BulletInst.h"
#include "EnemyInst.h"
#include "ItemInst.h"
#include "AnimatedInst.h"

#include "../../util/draw_util.h"
#include "../GameState.h"
#include "../../data/sprite_data.h"
#include "../../data/tile_data.h"
#include "../../display/display.h"
#include "../../data/item_data.h"
#include "TestInst.h"


static int scan_entrance(const std::vector<GameLevelPortal>& portals,
		const Pos& tilepos) {
	for (int i = 0; i < portals.size(); i++) {
		if (portals[i].entrancesqr == tilepos) {
			return i;
		}
	}
	return -1;
}

static bool item_hit(GameInst* self, GameInst* other) {
	return dynamic_cast<ItemInst*>(other) != NULL;
}

static bool enemy_hit(GameInst* self, GameInst* other) {
	return dynamic_cast<EnemyInst*>(other) != NULL;
}


void PlayerInst::perform_io_action(GameState* gs){
	GameView& view = gs->window_view();
	std::vector<GameAction> actions;
	int level = gs->branch_level();
	int frame = gs->frame();
	int dx = 0, dy = 0;
	bool mouse_within = gs->mouse_x() < gs->window_view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	bool item_used = false, spell_used = false;

	//Shifting target
	if (gs->key_press_state(SDLK_k)) {
		gs->monster_controller().shift_target(gs);
	}

	if (gs->key_press_state(SDLK_SPACE)){
		spellselect++;
		if (spellselect >= 2){
			spellselect = 0;
		}
	}

	//Resting
	bool resting = false;
	if (gs->key_down_state(SDLK_r) && canrestcooldown == 0) {
		actions.push_back(
				GameAction(id, GameAction::REST, frame, level)
		);
		resting = true;
	}

	if (!resting){
		//Arrow/wasd movement
		if (gs->key_down_state(SDLK_UP) || gs->key_down_state(SDLK_w)) {
			dy -= 1;
		}
		if (gs->key_down_state(SDLK_RIGHT) || gs->key_down_state(SDLK_d)) {
			dx += 1;
		}
		if (gs->key_down_state(SDLK_DOWN) || gs->key_down_state(SDLK_s)) {
			dy += 1;
		}
		if (gs->key_down_state(SDLK_LEFT) || gs->key_down_state(SDLK_a)) {
			dx -= 1;
		}
		if (dx != 0 || dy != 0){
			actions.push_back(
					GameAction(id, GameAction::MOVE_IN_DIRECTION, frame, level, 0, dx, dy)
			);
		}

		//Spell use
		if (gs->key_down_state(SDLK_j)){
			MonsterController& mc = gs->monster_controller();
			GameInst* target = gs->get_instance(mc.targetted);
			int mpcost = 10;
			if (spellselect) mpcost = 20;
			if (target && !stats().has_cooldown() && stats().mp >= mpcost &&
					!gs->solid_test(this) && gs->object_visible_test(this)){
				actions.push_back(
						GameAction(id, GameAction::SPELL, frame, level, spellselect, target->x, target->y)
				);
			}
		}
		if (gs->mouse_right_click() && mouse_within) {

			int px = x, py = y;
			x = rmx, y = rmy;
			if (!stats().has_cooldown() && stats().mp >= 50 &&
					!gs->solid_test(this) && gs->object_visible_test(this)){
				actions.push_back(
						GameAction(id, GameAction::SPELL, frame, level, 2, x, y)
				);
			}
			x = px, y = py;
		}

		if (gs->mouse_left_down() && mouse_within) {
			int mpcost = 10;
			if (spellselect) mpcost = 20;
			if (!stats().has_cooldown() && stats().mp >= mpcost &&
					!gs->solid_test(this) && gs->object_visible_test(this)){
				actions.push_back(
						GameAction(id, GameAction::SPELL, frame, level, spellselect, rmx, rmy)
				);
			}
		}

		//Item use
		for (int i = 0; i < 9; i++) {
			if (gs->key_press_state(SDLK_1 + i)) {
				if (inventory.inv[i].n > 0) {
					item_used = true;
					actions.push_back(
							GameAction(id, GameAction::USE_ITEM, frame, level, i)
					);
					break;//Can only use one item/step (until we have item cooldowns)
				}
			}
		}
		if (!item_used && gs->mouse_left_click() && !mouse_within) {
			int posx = (gs->mouse_x() - gs->window_view().width) / TILE_SIZE;
			int posy = (gs->mouse_y() - INVENTORY_POSITION) / TILE_SIZE;
			int slot = 5 * posy + posx;
			if (slot >= 0 && slot < INVENTORY_SIZE
					&& inventory.inv[slot].n > 0) {
				actions.push_back(
						GameAction(id, GameAction::USE_ITEM, frame, level, slot)
				);
			}
		}
	}

	//Item pickup
	GameInst* item = NULL;
	if (gs->object_radius_test(this, &item, 1, &item_hit)) {
		actions.push_back(
				GameAction(id, GameAction::PICKUP_ITEM, frame, level, item->id)
		);
	}

	Pos hitsqr;
	if (gs->key_down_state(SDLK_PERIOD) || gs->mouse_downwheel()) {
		if (gs->tile_radius_test(x, y, RADIUS, false, TILE_STAIR_DOWN,
				&hitsqr)) {
			int entr_n = scan_entrance(gs->level()->entrances, hitsqr);
			actions.push_back(
					GameAction(id, GameAction::USE_ENTRANCE, frame, level, entr_n)
			);
		}
	}
	if (gs->key_down_state(SDLK_COMMA) || gs->mouse_upwheel()) {
		if (gs->tile_radius_test(x, y, RADIUS, false, TILE_STAIR_UP,
				&hitsqr)) {
			int entr_n = scan_entrance(gs->level()->exits, hitsqr);
			actions.push_back(
					GameAction(id, GameAction::USE_EXIT, frame, level, entr_n)
			);
		}
	}


	for (int i = 0; i < actions.size(); i++){
		perform_action(gs, actions[i]);
	}
}

void PlayerInst::pickup_item(GameState* gs, const GameAction& action){
	ItemInst* item = (ItemInst*)gs->get_instance(action.use_id);
	gs->remove_instance(item);
	if (item->item_type() == ITEM_GOLD) {
		money += 10;
	} else {
		inventory.add(item->item_type(), 1);
	}
}

void PlayerInst::perform_action(GameState* gs, const GameAction& action){
	switch (action.act){
	case GameAction::MOVE_IN_DIRECTION:
		return use_move_and_melee(gs, action);
	case GameAction::SPELL:
		return use_spell(gs, action);
	case GameAction::REST:
		return use_rest(gs, action);
	case GameAction::USE_ENTRANCE:
		return use_dngn_entrance(gs, action);
	case GameAction::USE_EXIT:
		return use_dngn_exit(gs, action);
	case GameAction::USE_ITEM:
		return use_item(gs, action);
	case GameAction::PICKUP_ITEM:
		return pickup_item(gs, action);
	}
}

void PlayerInst::use_item(GameState *gs, const GameAction& action){
	int item = inventory.inv[action.use_id].item;
	game_item_data[item].action(this);
	inventory.inv[action.use_id].n--;

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
}
void PlayerInst::use_rest(GameState *gs, const GameAction& action){
	if (canrestcooldown == 0){
		stats().raise_hp(stats().hpregen * 8);
		stats().raise_mp(stats().mpregen * 8);
	}

}
void PlayerInst::use_move_and_melee(GameState* gs, const GameAction& action) {
	int dx = action.action_x;
	int dy = action.action_y;

	float mag = effective_stats().movespeed;
	float ddx = dx * mag;
	float ddy = dy * mag;


	EnemyInst* target = NULL;
	gs->object_radius_test(this, (GameInst**) &target, 1, &enemy_hit, x + ddx,
			y + ddy);

	EnemyInst* alreadyhitting[5] = { 0, 0, 0, 0, 0 };
	gs->object_radius_test(this, (GameInst**) alreadyhitting, 5, &enemy_hit, x,
			y);
	bool already = false;
	for (int i = 0; i < 5; i++) {
		if (alreadyhitting[i]) {
			if (ddx < 0 == ((alreadyhitting[i]->x - x + ddx * 2) < 0)) {
				ddx = 0;
			}
			if (ddy < 0 == ((alreadyhitting[i]->y - y + ddy * 2) < 0)) {
				ddy = 0;
			}
			already = true;
		}
	}

	gs->tile_radius_test(x + ddx, y + ddy, radius);

	if (!gs->tile_radius_test(x + ddx, y + ddy, radius)) {
		x += ddx;
		y += ddy;
	} else if (!gs->tile_radius_test(x + ddx, y, radius)) {
		x += ddx;
	} else if (!gs->tile_radius_test(x, y + ddy, radius)) {
		y += ddy;
	}

	if (ddx == 0 && ddy == 0){
		if (target && !stats().has_cooldown()) {
			int damage = effective_stats().melee.damage + gs->rng().rand(-4,5);
			if (target->hurt(gs, damage)) {
				stats().gain_xp(target->xpworth());
			}
			stats().reset_melee_cooldown(effective_stats());
			gs->add_instance(new AnimatedInst(target->x, target->y, SPR_SHORT_SWORD, 25));
			char dmgstr[32];
			snprintf(dmgstr, 32, "%d", damage);
			float rx, ry;
			direction_towards(Pos(x,y), Pos(target->x, target->y), rx, ry, 0.5);
			gs->add_instance(new AnimatedInst(target->x-5+rx*5, target->y-3+rx*5, -1, 25, rx, ry, dmgstr));

		}
	} else {
		canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN/4);
	}
}

void PlayerInst::use_dngn_exit(GameState* gs, const GameAction& action){
	int entr_n = action.use_id;
	LANARTS_ASSERT(
			entr_n >= 0 && entr_n < gs->level()->exits.size());
	portal = &gs->level()->exits[entr_n];
	gs->branch_level()--;
	gs->set_generate_flag();

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
}
void PlayerInst::use_dngn_entrance(GameState* gs, const GameAction& action){
	int entr_n = action.use_id;
	LANARTS_ASSERT(
			entr_n >= 0 && entr_n < gs->level()->entrances.size());
	portal = &gs->level()->entrances[entr_n];
	gs->branch_level()++;
	gs->set_generate_flag();

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
}
//void PlayerInst::use_dngn_entrance(GameState* gs, const GameAction& action){
//
//	//Up/down stairs
////	if (gs->key_down_state(SDLK_PERIOD) || gs->mouse_downwheel()) {
////				Pos hitsqr;
////				if (gs->tile_radius_test(x, y, RADIUS, false, TILE_STAIR_DOWN,
////						&hitsqr)) {
////					int entr_n = scan_entrance(gs->level()->entrances, hitsqr);
////					LANARTS_ASSERT(
////							entr_n >= 0 && entr_n < gs->level()->entrances.size());
////					portal = &gs->level()->entrances[entr_n];
////					gs->branch_level()++;
////					gs->set_generate_flag();
////				}
////			}
////			if ((gs->key_down_state(SDLK_COMMA) || gs->mouse_upwheel())
////					&& gs->branch_level() > 1) {
////				Pos hitsqr;
////				if (gs->tile_radius_test(x, y, RADIUS, false, TILE_STAIR_UP,
////						&hitsqr)) {
////					int entr_n = scan_entrance(gs->level()->exits, hitsqr);
////					LANARTS_ASSERT(
////							entr_n >= 0 && entr_n < gs->level()->entrances.size());
////					portal = &gs->level()->exits[entr_n];
////					gs->branch_level()--;
////					gs->set_generate_flag();
////				}
////			}
//
//			if ((gs->key_down_state(SDLK_COMMA) || gs->mouse_upwheel())
//					&& gs->branch_level() > 1) {
//				Pos hitsqr;
//				if (gs->tile_radius_test(x, y, RADIUS, false, TILE_STAIR_UP,
//						&hitsqr)) {
//					int entr_n = scan_entrance(gs->level()->exits, hitsqr);
//					LANARTS_ASSERT(
//							entr_n >= 0 && entr_n < gs->level()->entrances.size());
//					portal = &gs->level()->exits[entr_n];
//					gs->branch_level()--;
//					gs->set_generate_flag();
//				}
//			}
//
//
//	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
//}

void PlayerInst::use_spell(GameState* gs, const GameAction& action){

	if (action.use_id == 0){
		stats().mp -= 10;
	} else if (action.use_id == 1){
		stats().mp -= 20;
	} else if (action.use_id == 2){
		stats().mp -= 50;
	}

	Attack atk(effective_stats().ranged);
	bool bounce = true;
	int hits = 0;

	if (action.use_id == 1){
		atk.attack_sprite = SPR_MAGIC_BLAST;
		atk.projectile_speed /= 1.75;
		atk.damage *= 2;
		bounce = false;
		hits = 3;
	}

	if (action.use_id < 2){
		GameInst* bullet = new BulletInst(id, atk, x, y, action.action_x, action.action_y, bounce, hits);
		gs->add_instance(bullet);
	} else {
		x = action.action_x;
		y = action.action_y;
	}

	if (action.use_id == 1)
		base_stats.cooldown = effective_stats().ranged.cooldown*1.4;
	else if (action.use_id == 0){
		double mult = 1 + base_stats.xplevel/8.0;
		mult = std::min(2.0, mult);
		base_stats.cooldown = effective_stats().ranged.cooldown/mult;
	} else if (action.use_id == 2){
		base_stats.cooldown = effective_stats().ranged.cooldown*2;
	}

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
}
