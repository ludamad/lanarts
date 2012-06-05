/*
 * PlayerInstActions.cpp
 *  Implements the various actions that the player can perform, handles keyboard & mouse IO
 *  as well as networking communication of actions
 */

extern "C" {
#include <lua/lua.h>
}

#include "../../../data/item_data.h"
#include "../../../data/sprite_data.h"
#include "../../../data/tile_data.h"
#include "../../../data/weapon_data.h"

#include "../../../display/display.h"

#include "../../../lua/lua_api.h"

#include "../../../util/math_util.h"
#include "../../../util/collision_util.h"
#include "../../../util/game_basic_structs.h"

#include "../../GameState.h"

#include "../../utility_objects/AnimatedInst.h"

#include "../PlayerInst.h"
#include "../EnemyInst.h"
#include "../ItemInst.h"

#include "../ProjectileInst.h"

// static FILE* saved = fopen("res/saved_replay.rep", "wb");
// static FILE* open = fopen("res/replay.rep", "rb");
static std::vector<GameAction> replay_actions;

static bool is_similar_projectile(projectile_id projectile, item_id item) {
	if (projectile > -1) {
		ItemEntry& ientry = game_item_data[item];
		if (ientry.equipment_type == ItemEntry::PROJECTILE) {
			return (ientry.equipment_id == projectile);
		}
	}
	return false;
}

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

void PlayerInst::queue_io_equipment_actions(GameState* gs) {
	GameView& view = gs->window_view();
	bool mouse_within = gs->mouse_x() < gs->window_view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	int level = gs->level()->roomid;
	int frame = gs->frame();
	bool item_used = false;

	Pos p(gs->mouse_x() + view.x, gs->mouse_y() + view.y);
	obj_id target = gs->monster_controller().current_target();
	GameInst* targetted = gs->get_instance(target);
	if (targetted)
		p = Pos(targetted->x, targetted->y);

	//Item use
	for (int i = 0; i < 9; i++) {
		if (gs->key_press_state(SDLK_1 + i)) {
			if (get_inventory().get(i).amount > 0) {
				item_used = true;
				queued_actions.push_back(
						GameAction(id, GameAction::USE_ITEM, frame, level, i,
								p.x, p.y));
				break; //Can only use one item/step (until we have item cooldowns)
			}
		}
	}

	//Item pickup
	GameInst* item = NULL;
	if (cooldowns.can_pickup()
			&& gs->object_radius_test(this, &item, 1, &item_colfilter)) {
		ItemInst* iteminst = (ItemInst*)item;
		item_id type = iteminst->item_type();
		bool autopickup = game_item_data[type].equipment_type == ItemEntry::NONE
				&& iteminst->last_held_by() != id;
		autopickup |= is_similar_projectile(equipment.projectile, type);
		bool pickup_io = gs->key_down_state(SDLK_LSHIFT)
				|| gs->key_down_state(SDLK_RSHIFT);
		if (pickup_io || autopickup)
			queued_actions.push_back(
					GameAction(id, GameAction::PICKUP_ITEM, frame, level,
							item->id));
	}

	if (!item_used) {
		gs->game_hud().queue_io_actions(gs, this, queued_actions);
	}
}
void PlayerInst::queue_io_actions(GameState* gs) {
	GameView& view = gs->window_view();
	int level = gs->level()->roomid;
	int frame = gs->frame();
	int dx = 0, dy = 0;
	bool mouse_within = gs->mouse_x() < gs->window_view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	if (is_local_player()) {
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
		if (gs->key_down_state(SDLK_r) && cooldowns.can_rest()) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_REST, frame, level));
			resting = true;
		}

		if (!resting) {
			queue_io_spell_and_attack_actions(gs, dx, dy);
			queue_io_equipment_actions(gs);

			if (gs->key_down_state(SDLK_PERIOD) || gs->mouse_downwheel()) {
				Pos hitsqr;
				if (gs->tile_radius_test(x, y, RADIUS, false,
						get_tile_by_name("stairs_down"), &hitsqr)) {
					queued_actions.push_back(
							GameAction(id, GameAction::USE_ENTRANCE, frame,
									level));
				}
			}
			if (gs->key_down_state(SDLK_COMMA) || gs->mouse_upwheel()) {
				Pos hitsqr;
				if (gs->tile_radius_test(x, y, RADIUS, false,
						get_tile_by_name("stairs_up"), &hitsqr)) {
					queued_actions.push_back(
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
				queued_actions.push_back(
						GameAction(id, GameAction::MOVE, frame, level, 0, dx,
								dy));
			}
		}

	}
	GameNetConnection& connection = gs->net_connection();
	bool hasconnection = connection.get_connection() != NULL;
	NetPacket packet;

	if (is_local_player() && hasconnection) {
		for (int i = 0; i < queued_actions.size(); i++) {
			queued_actions[i].frame = gs->frame();
			packet.add(queued_actions[i]);
		}
		packet.encode_header();
		connection.get_connection()->broadcast_packet(packet);
	}
}

void PlayerInst::pickup_item(GameState* gs, const GameAction& action) {
	const int PICKUP_RATE = 10;
	ItemInst* item = (ItemInst*)gs->get_instance(action.use_id);
	item_id type = item->item_type();
	int amnt = item->item_quantity();

	if (!item)
		return;
	if (type == get_item_by_name("Gold")) {
		money += amnt;
	} else {
		if (is_similar_projectile(equipment.projectile, type))
			equipment.projectile_amnt += amnt;
		else
			get_inventory().add(type, amnt);
	}

	cooldowns.canpickupcooldown = PICKUP_RATE;
	gs->remove_instance(item);
}

void PlayerInst::queue_network_actions(GameState *gs) {
	GameNetConnection& connection = gs->net_connection();
	bool hasconnection = connection.get_connection() != NULL;
	NetPacket packet;
	if (!is_local_player() && hasconnection) {
		int tries = 0;
		while (!connection.get_connection()->get_next_packet(packet)) {
			if ((++tries) % 30000 == 0) {
				if (!gs->update_iostate(false)) {
					exit(0);
				}
			}
		}
		bool output1 = true;
		while (packet.body_length > 0) {
			GameAction action;
			packet.get(action);
			if (output1 && action.frame != gs->frame()) {
				printf("action frame %d vs %d\n", action.frame, gs->frame());
				output1 = false;
			}
			queued_actions.push_front(action);
		}
	}
}

void PlayerInst::perform_queued_actions(GameState *gs) {
	for (int i = 0; i < queued_actions.size(); i++) {
// 		to_action_file(saved, actions[i]);
		perform_action(gs, queued_actions[i]);
	}
	queued_actions.clear();
}

void PlayerInst::equip(item_id item, int amnt) {
	equipment.equip(item, amnt);
}

void PlayerInst::drop_item(GameState* gs, const GameAction& action) {
// 	ItemInst* item = (ItemInst*) gs->get_instance(action.use_id);
	ItemSlot& itemslot = get_inventory().get(action.use_id);
	int dropx = round_to_multiple(x, TILE_SIZE, true), dropy =
			round_to_multiple(y, TILE_SIZE, true);
	int amnt = itemslot.amount;
	gs->add_instance(new ItemInst(itemslot.item, dropx, dropy, amnt, id));
	itemslot.amount -= amnt;
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
	case GameAction::DEEQUIP_ITEM:
		return equipment.deequip(action.use_id);
	default:
		printf("PlayerInst::perform_action() error: Invalid action id %d!\n",
				action.act);
		break;
	}
}

static bool item_check_lua_prereq(lua_State* L, ItemEntry& type, obj_id user) {
	if (type.prereq_func.empty())
		return true;

	type.prereq_func.push(L);
	luayaml_push_item(L, type.name.c_str());
	lua_pushgameinst(L, user);
	lua_call(L, 2, 1);

	bool ret = lua_toboolean(L, lua_gettop(L));
	lua_pop(L, 1);

	return ret;
}
static void item_do_lua_action(lua_State* L, ItemEntry& type, obj_id user,
		const Pos& p, int amnt) {
	type.action_func.push(L);
	luayaml_push_item(L, type.name.c_str());
	lua_pushgameinst(L, user);
	lua_pushnumber(L, p.x);
	lua_pushnumber(L, p.y);
	lua_pushnumber(L, amnt);
	lua_call(L, 5, 0);
}
void PlayerInst::use_item(GameState *gs, const GameAction& action) {
	ItemSlot& itemslot = get_inventory().get(action.use_id);
	ItemEntry& type = game_item_data[itemslot.item];

	lua_State* L = gs->get_luastate();

	if (itemslot.amount > 0 && equipment.valid_to_use(itemslot.item)
			&& item_check_lua_prereq(L, type, this->id)) {
		item_do_lua_action(L, type, this->id,
				Pos(action.action_x, action.action_y), itemslot.amount);
		if (type.equipment_type == ItemEntry::PROJECTILE)
			itemslot.amount = 0;
		else
			itemslot.amount--;
		reset_rest_cooldown();
	}

}
void PlayerInst::use_rest(GameState *gs, const GameAction& action) {
	bool atfull = stats().hp >= stats().max_hp && stats().mp >= stats().max_mp;
	if (cooldowns.can_rest() && !atfull) {
		stats().raise_hp(stats().hpregen * 8);
		stats().raise_mp(stats().mpregen * 8);
		isresting = true;
	}
}
void PlayerInst::use_move(GameState* gs, const GameAction& action) {
	int dx = action.action_x;
	int dy = action.action_y;

	float mag = effective_stats(gs->get_luastate()).movespeed;

	float ddx = dx * mag;
	float ddy = dy * mag;

	EnemyInst* target = NULL;
	//Enemy hitting test for melee
	gs->object_radius_test(this, (GameInst**)&target, 1, &enemy_colfilter,
			x + ddx * 2, y + ddy * 2);

	//Smaller radius enemy pushing test, can intercept enemy radius but not too far
	EnemyInst* alreadyhitting[5] = { 0, 0, 0, 0, 0 };
	gs->object_radius_test(this, (GameInst**)alreadyhitting, 5,
			&enemy_colfilter, x, y, radius);
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
	bool didhit = gs->tile_radius_test(x, y, radius, false,
			get_tile_by_name("stairs_up"), &hitpos);
	int entr_n = scan_entrance(gs->level()->exits, hitpos);
	if (!didhit || entr_n == -1)
		return;

	LANARTS_ASSERT( entr_n >= 0 && entr_n < gs->level()->exits.size());
	gs->ensure_connectivity(gs->level()->roomid - 1, gs->level()->roomid);
	GameLevelPortal* portal = &gs->level()->exits[entr_n];

	int px = centered_multiple(portal->exitsqr.x, TILE_SIZE);
	int py = centered_multiple(portal->exitsqr.y, TILE_SIZE);
	gs->level_move(id, px, py, gs->level()->roomid, gs->level()->roomid - 1);

	reset_rest_cooldown();
}
void PlayerInst::use_dngn_entrance(GameState* gs, const GameAction& action) {
	Pos hitpos;
	bool didhit = gs->tile_radius_test(x, y, radius, false,
			get_tile_by_name("stairs_down"), &hitpos);
	int entr_n = scan_entrance(gs->level()->entrances, hitpos);
	if (!didhit || entr_n == -1)
		return;

	LANARTS_ASSERT( entr_n >= 0 && entr_n < gs->level()->entrances.size());
	gs->ensure_connectivity(gs->level()->roomid, gs->level()->roomid + 1);
	GameLevelPortal* portal = &gs->level()->entrances[entr_n];

	int px = centered_multiple(portal->exitsqr.x, TILE_SIZE);
	int py = centered_multiple(portal->exitsqr.y, TILE_SIZE);
	gs->level_move(id, px, py, gs->level()->roomid, gs->level()->roomid + 1);

	reset_rest_cooldown();
}

void PlayerInst::gain_xp(GameState* gs, int xp) {
	int levels_gained = stats().gain_xp(xp);
	if (levels_gained > 0) {
//		bool plural = levels_gained > 1;

		char level_gain_str[128];
		snprintf(level_gain_str, 128, "You have reached level %d!",
				stats().xplevel);
		gs->game_chat().add_message(level_gain_str, Colour(50, 205, 50));
	}
}
