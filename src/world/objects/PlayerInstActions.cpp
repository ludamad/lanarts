/*
 * PlayerInstActions.cpp
 *  Implements the various actions that the player can perform, handles keyboard & mouse IO
 *  as well as networking communication of actions
 */

#include "PlayerInst.h"
#include "ProjectileInst.h"
#include "EnemyInst.h"
#include "ItemInst.h"
#include "AnimatedInst.h"

#include "../../util/draw_util.h"
#include "../../util/math_util.h"
#include "../../util/collision_util.h"
#include "../../util/game_basic_structs.h"

#include "../GameState.h"
#include "../../data/sprite_data.h"
#include "../../data/tile_data.h"
#include "../../display/display.h"
#include "../../data/item_data.h"
#include "../../data/weapon_data.h"
#include "TestInst.h"
#include <deque>

// static FILE* saved = fopen("res/saved_replay.rep", "wb");
// static FILE* open = fopen("res/replay.rep", "rb");
static std::vector<GameAction> replay_actions;

static void get_current_actions(int frame, std::vector<GameAction>& actions) {
// 	if (!open)
// 		return;
// 	if (!feof(open)) {
// 		while (!feof(open)) {
// 			replay_actions.push_back(from_action_file(open));
// 			if (replay_actions.back().frame > frame)
// 				break;
// 		}
// 	}
// 	int i;
// 	for (i = 0; i < replay_actions.size(); i++) {
// 		if (replay_actions[i].frame == frame) {
// 			actions.push_back(replay_actions[i]);
// 		} else if (replay_actions[i].frame > frame)
// 			break;
// 	}
// 	replay_actions = std::vector<GameAction>(&replay_actions[i], &replay_actions[replay_actions.size()]);
}

void PlayerInst::perform_io_action(GameState* gs) {
	GameView& view = gs->window_view();
	std::deque<GameAction> actions;
	int level = gs->level()->roomid;
	int frame = gs->frame();
	int dx = 0, dy = 0;
	bool mouse_within = gs->mouse_x() < gs->window_view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	bool item_used = false, spell_used = false;

	if (is_local_focus()) {
		//Shifting target
		if (gs->key_press_state(SDLK_k)) {
			gs->monster_controller().shift_target(gs);
		}

		if (gs->key_press_state(SDLK_SPACE)) {
			spellselect++;
			if (spellselect > 1) {
				spellselect = 0;
			}
		}
		if (gs->key_press_state(SDLK_m))
			spellselect = -1;

//		get_current_actions(gs->frame(), actions);

//Resting
		bool resting = false;
		if (gs->key_down_state(SDLK_r) && canrestcooldown == 0) {
			actions.push_back(GameAction(id, GameAction::USE_REST, frame, level));
			resting = true;
		}

		if (!resting) {

			//Spell use
			if (gs->key_down_state(SDLK_j)) {
				MonsterController& mc = gs->monster_controller();
				GameInst* target = gs->get_instance(mc.current_target());
				int mpcost = 10;
				if (spellselect)
					mpcost = 20;
				if (target && (spellselect == -1 || stats().mp < mpcost)){
					actions.push_back(
							GameAction(id, GameAction::USE_WEAPON, frame, level,
									spellselect, target->x, target->y));
				} else {
					if (target && !stats().has_cooldown() && stats().mp >= mpcost
							&& gs->object_visible_test(target, this)) {
						actions.push_back(
								GameAction(id, GameAction::USE_SPELL, frame, level,
										spellselect, target->x, target->y));
					}
				}
			}
			if (gs->mouse_right_click() && mouse_within) {

				int px = x, py = y;
				x = rmx, y = rmy;
				if (stats().mp >= 50 && !gs->solid_test(this)
						&& gs->object_visible_test(this)) {
					actions.push_back(
							GameAction(id, GameAction::USE_SPELL, frame, level, 2,
									x, y));
				}
				x = px, y = py;
			}

			if (gs->mouse_left_down() && mouse_within) {
				int mpcost = 10;
				if (spellselect)
					mpcost = 20;
				if (spellselect == -1 || stats().mp < mpcost){
					actions.push_back(
							GameAction(id, GameAction::USE_WEAPON, frame, level,
									spellselect, rmx, rmy));
				} else if (!stats().has_cooldown() && stats().mp >= mpcost) {
					actions.push_back(
							GameAction(id, GameAction::USE_SPELL, frame, level,
									spellselect, rmx, rmy));
				}
			}

			//Item use
			for (int i = 0; i < 9; i++) {
				if (gs->key_press_state(SDLK_1 + i)) {
					if (inventory.inv[i].n > 0) {
						item_used = true;
						actions.push_back(
								GameAction(id, GameAction::USE_ITEM, frame,
										level, i));
						break; //Can only use one item/step (until we have item cooldowns)
					}
				}
			}
			if (!item_used && gs->mouse_left_click() && !mouse_within) {
				int posx = (gs->mouse_x() - gs->window_view().width)
						/ TILE_SIZE;
				int posy = (gs->mouse_y() - INVENTORY_POSITION) / TILE_SIZE;
				int slot = 5 * posy + posx;
				if (slot >= 0 && slot < INVENTORY_SIZE
						&& inventory.inv[slot].n > 0) {
					actions.push_back(
							GameAction(id, GameAction::USE_ITEM, frame, level,
									slot));
				}
			}
			// Drop item
			if (!item_used && gs->mouse_right_click() && !mouse_within) {
				int posx = (gs->mouse_x() - gs->window_view().width)
						/ TILE_SIZE;
				int posy = (gs->mouse_y() - INVENTORY_POSITION) / TILE_SIZE;
				int slot = 5 * posy + posx;
				if (slot >= 0 && slot < INVENTORY_SIZE
						&& inventory.inv[slot].n > 0) {
					actions.push_back(
							GameAction(id, GameAction::DROP_ITEM, frame, level,
									slot));
				}
			}
		}

		//Item pickup
		GameInst* item = NULL;
		if (gs->object_radius_test(this, &item, 1, &item_colfilter)) {
			ItemInst* iteminst = (ItemInst*) item;
			int type = iteminst->item_type();
			bool autopickup = game_item_data[type].weapon < 0
					&& iteminst->last_held_by() != id;
			bool pickup_io = gs->key_down_state(SDLK_LSHIFT)
					|| gs->key_down_state(SDLK_RSHIFT);
			if (pickup_io || autopickup)
				actions.push_back(
						GameAction(id, GameAction::PICKUP_ITEM, frame, level,
								item->id));
		}

		Pos hitsqr;
		if (gs->key_down_state(SDLK_PERIOD) || gs->mouse_downwheel()) {
			if (gs->tile_radius_test(x, y, RADIUS, false,
					get_tile_by_name("stairs_down"), &hitsqr)) {
//				int entr_n = scan_entrance(gs->level()->entrances, hitsqr);
				actions.push_back(
						GameAction(id, GameAction::USE_ENTRANCE, frame, level));
			}
		}
		if (gs->key_down_state(SDLK_COMMA) || gs->mouse_upwheel()) {
			if (gs->tile_radius_test(x, y, RADIUS, false,
					get_tile_by_name("stairs_up"), &hitsqr)) {
//				int entr_n = scan_entrance(gs->level()->exits, hitsqr);
				actions.push_back(
						GameAction(id, GameAction::USE_EXIT, frame, level));
			}
		}
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
		if (dx != 0 || dy != 0) {
			actions.push_back(
					GameAction(id, GameAction::MOVE, frame, level,
							0, dx, dy));
		}
	}
	GameNetConnection& connection = gs->net_connection();
	bool hasconnection = connection.get_connection() != NULL;
	NetPacket packet;

	if (is_local_focus() && hasconnection) {
		for (int i = 0; i < actions.size(); i++) {
			actions[i].frame = gs->frame();
			packet.add(actions[i]);
		}
		packet.encode_header();
		connection.get_connection()->broadcast_packet(packet);
	}

	if (!is_local_focus() && hasconnection) {
		bool has_connect = false;
        int tries = 0;
		while (true) {
			if (connection.get_connection()->get_next_packet(packet)) {
                
				has_connect = true;
				break;
			} else if ((++tries)%30000 == 0){
                if ( !gs->update_iostate() ) {
                    exit(0);
                }
        }
//				if (gs->game_settings().conntype == GameSettings::HOST)
//					break;
		}
		if (has_connect) {
			bool output1 = true;
			while (packet.body_length > 0) {
				GameAction action;
//				LANARTS_ASSERT(action.frame == gs->frame());
				packet.get(action);
				if (output1 && action.frame != frame) {
					printf("action frame %d vs %d\n", action.frame, frame);
					output1 = false;
				}
				actions.push_front(action);
			}
		}
	}

	isresting = false;
	for (int i = 0; i < actions.size(); i++) {
// 		to_action_file(saved, actions[i]);
		perform_action(gs, actions[i]);
	}
}

void PlayerInst::pickup_item(GameState* gs, const GameAction& action) {
	ItemInst* item = (ItemInst*) gs->get_instance(action.use_id);
	if (!item) return;
	gs->remove_instance(item);
	if (item->item_type() == get_item_by_name("Gold")) {
		money += 10;
	} else {
		inventory.add(item->item_type(), 1);
	}
}

void PlayerInst::drop_item(GameState* gs, const GameAction& action) {
// 	ItemInst* item = (ItemInst*) gs->get_instance(action.use_id);
	itemslot& item = inventory.inv[action.use_id];
	gs->add_instance(new ItemInst(item.item, x, y, id));
	inventory.inv[action.use_id].n--;
}

void PlayerInst::perform_action(GameState* gs, const GameAction& action) {
	switch (action.act) {
	case GameAction::MOVE:
		return use_move(gs, action);
	case GameAction::USE_WEAPON:
		return use_weapon(gs, action);
	case GameAction::USE_SPELL:
		return use_spell(gs, action);
	case GameAction::USE_REST:
		return use_rest(gs, action);
	case GameAction::USE_ENTRANCE:
		return use_dngn_entrance(gs, action);
	case GameAction::USE_EXIT:
		return use_dngn_exit(gs, action);
	case GameAction::USE_ITEM:
		return use_item(gs, action);
	case GameAction::PICKUP_ITEM:
		return pickup_item(gs, action);
	case GameAction::DROP_ITEM:
		return drop_item(gs, action);
	}
}

void PlayerInst::use_item(GameState *gs, const GameAction& action) {
	int item = inventory.inv[action.use_id].item;
	game_item_data[item].action(gs, &game_item_data[item], this);
	inventory.inv[action.use_id].n--;

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
}
void PlayerInst::use_rest(GameState *gs, const GameAction& action) {
	bool atfull = stats().hp >= stats().max_hp && stats().mp >= stats().max_mp;
	if (canrestcooldown == 0 && !atfull) {
		stats().raise_hp(stats().hpregen * 8);
		stats().raise_mp(stats().mpregen * 8);
		isresting = true;
	}
}
void PlayerInst::use_move(GameState* gs, const GameAction& action) {
	int dx = action.action_x;
	int dy = action.action_y;

	float mag = effective_stats().movespeed;

	float ddx = dx * mag;
	float ddy = dy * mag;

	EnemyInst* target = NULL;
	//Enemy hitting test for melee
	gs->object_radius_test(this, (GameInst**) &target, 1, &enemy_colfilter,
			x + ddx * 2, y + ddy * 2);

	//Smaller radius enemy pushing test, can intercept enemy radius but not too far
	EnemyInst* alreadyhitting[5] = { 0, 0, 0, 0, 0 };
	gs->object_radius_test(this, (GameInst**) alreadyhitting, 5, &enemy_colfilter, x,
			y, radius);
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
}

static int scan_entrance(const std::vector<GameLevelPortal>& portals,
		const Pos& tilepos) {
	for (int i = 0; i < portals.size(); i++) {
		if (portals[i].entrancesqr == tilepos) {
			return i;
		}
	}
	return -1;
}

void PlayerInst::use_dngn_exit(GameState* gs, const GameAction& action) {
	Pos hitpos;
	LANARTS_ASSERT(
			gs->tile_radius_test(x,y,radius,false, get_tile_by_name("stairs_up"),&hitpos));
	int entr_n = scan_entrance(gs->level()->exits, hitpos);
//	int entr_n = action.use_id;
	LANARTS_ASSERT( entr_n >= 0 && entr_n < gs->level()->exits.size());
	gs->ensure_connectivity(gs->level()->roomid - 1, gs->level()->roomid);
	GameLevelPortal* portal = &gs->level()->exits[entr_n];
//
	int px = (portal->exitsqr.x) * TILE_SIZE + TILE_SIZE / 2;
	int py = (portal->exitsqr.y) * TILE_SIZE + TILE_SIZE / 2;
	gs->level_move(id, px, py, gs->level()->roomid, gs->level()->roomid - 1);

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
}
void PlayerInst::use_dngn_entrance(GameState* gs, const GameAction& action) {
	Pos hitpos;
	LANARTS_ASSERT(
			gs->tile_radius_test(x,y,radius,false, get_tile_by_name("stairs_down"),&hitpos));
	int entr_n = scan_entrance(gs->level()->entrances, hitpos);
//	int entr_n = action.use_id;
	LANARTS_ASSERT( entr_n >= 0 && entr_n < gs->level()->entrances.size());
	gs->ensure_connectivity(gs->level()->roomid, gs->level()->roomid + 1);
	GameLevelPortal* portal = &gs->level()->entrances[entr_n];
//
	int px = (portal->exitsqr.x) * TILE_SIZE + TILE_SIZE / 2;
	int py = (portal->exitsqr.y) * TILE_SIZE + TILE_SIZE / 2;
	gs->level_move(id, px, py, gs->level()->roomid, gs->level()->roomid + 1);
	if (is_local_focus()) {

	}
//	gs->remove_instance(this, true);

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
}

void PlayerInst::use_spell(GameState* gs, const GameAction& action) {
    if (action.use_id < 2 && stats().has_cooldown()) return;

	if (action.use_id == 0) {
		stats().mp -= 10;
	} else if (action.use_id == 1) {
		stats().mp -= 20;
	} else if (action.use_id == 2) {
		stats().mp -= 50;
	}

	Attack atk(effective_stats().ranged);
	bool bounce = true;
	int hits = 0;

	if (action.use_id == 1) {
		atk.attack_sprite = get_sprite_by_name("magic blast"); //SPR_MAGIC_BLAST;
		atk.projectile_speed /= 1.75;
		//	atk.damage *= 2;
		bounce = false;
		hits = 3;
	}
	atk.damage = effective_stats().calculate_spell_damage(gs->rng(),
			action.use_id);

	if (action.use_id < 2) {
		GameInst* bullet = new ProjectileInst(id, atk, x, y, action.action_x,
				action.action_y, bounce, hits);
		gs->add_instance(bullet);
	} else {
		x = action.action_x;
		y = action.action_y;
	}

	if (action.use_id == 1)
		base_stats.cooldown = effective_stats().ranged.cooldown * 1.4;
	else if (action.use_id == 0) {
		double mult = 1 + base_stats.xplevel / 8.0;
		mult = std::min(2.0, mult);
		base_stats.cooldown = effective_stats().ranged.cooldown / mult;
	} else if (action.use_id == 2) {
		base_stats.cooldown = effective_stats().ranged.cooldown * 2;
	}

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
}

void PlayerInst::use_weapon(GameState *gs, const GameAction& action) {
	const int MAX_MELEE_HITS = 10;

	if (stats().has_cooldown())
		return;

	WeaponType& weap = game_weapon_data[weapon];

	int dx = action.action_x - x, dy = action.action_y - y;
	float mag = sqrt(dx * dx + dy * dy);
	if (mag == 0) mag = 1;
	int ax = x + weap.range*dx/mag, ay = y + weap.range*dy/mag;

	GameInst* enemies[MAX_MELEE_HITS];

	int max_targets = std::min(MAX_MELEE_HITS, weap.max_targets);

	int numhit = gs->object_radius_test(this, enemies, MAX_MELEE_HITS, enemy_colfilter, ax,
			ay, weap.dmgradius);

	for (int i = 0; i < numhit; i++) {
		EnemyInst* e = (EnemyInst*) enemies[i];
		//int damage = effective_stats().melee.damage + gs->rng().rand(-4, 5);
		WeaponType& wtype = game_weapon_data[weapon_type()];
		int damage = effective_stats().calculate_melee_damage(gs->rng(),
				weapon_type());
		char buffstr[32];
		snprintf(buffstr, 32, "%d", damage);
		float rx, ry;
		direction_towards(Pos(x, y), Pos(e->x, e->y), rx, ry, 0.5);
		gs->add_instance(
				new AnimatedInst(e->x - 5 + rx * 5, e->y - 3 + rx * 5, -1, 25,
						rx, ry, buffstr));

		if (e->hurt(gs, damage)) {
			stats().gain_xp(e->xpworth());
			if ( is_local_focus() ){
				snprintf(buffstr, 32, "%d XP", e->xpworth());
				gs->add_instance(
						new AnimatedInst(e->x - 5, e->y - 5, -1, 25, 0, 0, buffstr,
								Colour(255, 215, 11)));
			}
		}
		stats().cooldown = wtype.cooldown;
		//stats().reset_melee_cooldown(effective_stats());
		int atksprite = game_weapon_data[weapon_type()].attack_sprite;
		gs->add_instance(new AnimatedInst(e->x, e->y, atksprite, 25));

	}
}
