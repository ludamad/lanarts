/*
 * PlayerInstActions.cpp
 *  Implements the various actions that the player can perform, handles keyboard & mouse IO
 *  as well as networking communication of actions
 */

extern "C" {
#include <lua/lua.h>
}

#include "../../display/display.h"
#include "../../display/sprite_data.h"
#include "../../display/tile_data.h"
#include "../../gamestate/GameState.h"
#include "../../lua/lua_api.h"

#include "../../stats/items/ItemEntry.h"

#include "../../stats/items/ProjectileEntry.h"
#include "../../stats/items/WeaponEntry.h"

#include "../../util/game_replays.h"
#include "../../util/math_util.h"

#include "../../lanarts_defines.h"

#include "../enemy/EnemyInst.h"

#include "../store/StoreInst.h"

#include "../AnimatedInst.h"
#include "../ItemInst.h"
#include "../ProjectileInst.h"
#include "../collision_filters.h"

#include "PlayerInst.h"

static bool is_wieldable_projectile(EquipmentStats& equipment,
		const Item& item) {
	if (equipment.projectile().is_same_item(item)) {
		return true;
	}

	if (item.is_projectile()) {
		ProjectileEntry& pentry = item.projectile_entry();
		if (pentry.weapon_class == "unarmed")
			return false;
	}

	if (!item.is_projectile())
		return false;

	return equipment.valid_to_use(item);
}

const int AUTOUSE_HEALTH_POTION_THRESHOLD = 20;

void PlayerInst::enqueue_io_equipment_actions(GameState* gs,
		bool do_stopaction) {
	GameView& view = gs->view();
	bool mouse_within = gs->mouse_x() < gs->view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	int level = gs->get_level()->id();
	int frame = gs->frame();
	bool item_used = false;
	IOController& io = gs->io_controller();

	Pos p(gs->mouse_x() + view.x, gs->mouse_y() + view.y);
	obj_id target = this->current_target;
	GameInst* targetted = gs->get_instance(target);
	if (targetted)
		p = Pos(targetted->x, targetted->y);

	// We may have already used an item eg due to auto-use of items
	bool used_item = false;

	//Item use
	for (int i = 0; i < 9 && !used_item; i++) {
		if (io.query_event(IOEvent(IOEvent::USE_ITEM_N, i))) {
			if (inventory().get(i).amount() > 0) {
				item_used = true;
				queued_actions.push_back(
						GameAction(id, GameAction::USE_ITEM, frame, level, i,
								p.x, p.y));
			}
		}
	}
	if (!used_item && gs->game_settings().autouse_health_potions
			&& core_stats().hp < AUTOUSE_HEALTH_POTION_THRESHOLD) {
		int item_slot = inventory().find_slot(
				get_item_by_name("Health Potion"));
		if (item_slot > -1) {
			queued_actions.push_back(
					game_action(gs, this, GameAction::USE_ITEM, item_slot));
			used_item = true;
		}
	}

	//Item pickup
	GameInst* inst = NULL;
	if (cooldowns().can_pickup()
			&& gs->object_radius_test(this, &inst, 1, &item_colfilter)) {
		ItemInst* iteminst = (ItemInst*)inst;
		Item& item = iteminst->item_type();

		bool was_dropper = iteminst->last_held_by() == id;
		bool dropper_autopickup = iteminst->autopickup_held();

		bool autopickup = (item.is_normal_item() && !was_dropper
				&& !dropper_autopickup) || (was_dropper && dropper_autopickup);

		bool wieldable_projectile = is_wieldable_projectile(equipment(), item);

		bool pickup_io = gs->key_down_state(SDLK_LSHIFT)
				|| gs->key_down_state(SDLK_RSHIFT);

		if (do_stopaction || wieldable_projectile || pickup_io || autopickup)
			queued_actions.push_back(
					GameAction(id, GameAction::PICKUP_ITEM, frame, level,
							iteminst->id));
	}
}
void PlayerInst::enqueue_io_movement_actions(GameState* gs, int& dx, int& dy) {
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
				game_action(gs, this, GameAction::MOVE, 0, dx, dy));
		moving = true;
	} else {
		moving = false;
	}
}

void PlayerInst::enqueue_actions(const ActionQueue& queue) {
	if (!actions_set_for_turn) {
		LANARTS_ASSERT(queued_actions.empty());
		queued_actions = queue;
		actions_set_for_turn = true;
	}
}

void PlayerInst::enqueue_io_actions(GameState* gs) {
	LANARTS_ASSERT(is_local_player() && gs->local_player() == this);

	if (actions_set_for_turn) {
		return;
	}

	actions_set_for_turn = true;

	GameSettings& settings = gs->game_settings();
	GameView& view = gs->view();

	PlayerDataEntry& pde = gs->player_data().local_player_data();

	if (pde.action_queue.has_actions_for_frame(gs->frame())) {
		pde.action_queue.extract_actions_for_frame(queued_actions, gs->frame());
		return;
	}
	gs->set_repeat_actions_counter(settings.frame_action_repeat);

	int dx = 0, dy = 0;
	bool mouse_within = gs->mouse_x() < gs->view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	bool was_moving = moving, do_stopaction = false;
	IOController& io = gs->io_controller();

	if (!settings.loadreplay_file.empty()) {
		load_actions(gs, queued_actions);
	} else if (is_local_player()) {
		enqueue_io_movement_actions(gs, dx, dy);

		if (was_moving && !moving && cooldowns().can_do_stopaction()) {
			do_stopaction = true;
		}
	}
	//Shifting target
	if (gs->key_press_state(SDLK_k)) {
		shift_autotarget(gs);
	}

	if (gs->key_press_state(SDLK_m))
		spellselect = -1;

	bool attack_used = false;
	if (!gs->game_hud().handle_io(gs, queued_actions)) {
		attack_used = enqueue_io_spell_and_attack_actions(gs, dx, dy);
		enqueue_io_equipment_actions(gs, do_stopaction);
	}

	bool action_usage = io.query_event(IOEvent::ACTIVATE_SPELL_N)
			|| io.query_event(IOEvent::USE_WEAPON)
			|| io.query_event(IOEvent::AUTOTARGET_CURRENT_ACTION)
			|| io.query_event(IOEvent::MOUSETARGET_CURRENT_ACTION);
	if ((do_stopaction && !action_usage) || gs->key_down_state(SDLK_PERIOD)
			|| gs->mouse_downwheel()) {
		Pos hitsqr;
		if (gs->tile_radius_test(x, y, RADIUS, false,
				get_tile_by_name("stairs_down"), &hitsqr)) {
			queued_actions.push_back(
					game_action(gs, this, GameAction::USE_ENTRANCE));
			cooldowns().reset_stopaction_timeout(50);
		}
	}

	if ((do_stopaction && !action_usage) || gs->key_down_state(SDLK_COMMA)
			|| gs->mouse_upwheel()) {
		Pos hitsqr;
		if (gs->tile_radius_test(x, y, RADIUS, false,
				get_tile_by_name("stairs_up"), &hitsqr)) {
			queued_actions.push_back(
					game_action(gs, this, GameAction::USE_EXIT));
			cooldowns().reset_stopaction_timeout(50);
		}
	}

	// If we haven't done anything, rest
	if (queued_actions.empty()) {
		queued_actions.push_back(game_action(gs, this, GameAction::USE_REST));
	}

	ActionQueue only_passive_actions;

	for (int i = 0; i < queued_actions.size(); i++) {
		GameAction::action_t act = queued_actions[i].act;
		if (act == GameAction::MOVE || act == GameAction::USE_REST) {
			only_passive_actions.push_back(queued_actions[i]);
		}
	}

	GameNetConnection& net = gs->net_connection();
	if (net.is_connected()) {
		net_send_player_actions(net, gs->frame(),
				player_get_playernumber(gs, this), queued_actions);
	}

	for (int i = 1; i <= settings.frame_action_repeat; i++) {
		for (int j = 0; j < only_passive_actions.size(); j++) {
			only_passive_actions[j].frame = gs->frame() + i;
		}
		pde.action_queue.queue_actions_for_frame(only_passive_actions,
				gs->frame() + i);

		if (net.is_connected()) {
			net_send_player_actions(net, gs->frame() + i,
					player_get_playernumber(gs, this), only_passive_actions);
		}
	}

}

void PlayerInst::pickup_item(GameState* gs, const GameAction& action) {
	const int PICKUP_RATE = 10;
	GameInst* inst = gs->get_instance(action.use_id);
	if (!inst) {
		return;
	}
	ItemInst* iteminst = dynamic_cast<ItemInst*>(inst);
	LANARTS_ASSERT(iteminst);

	const Item& type = iteminst->item_type();
	int amnt = iteminst->item_quantity();

	bool equip_as_well = false;

	if (type.id == get_item_by_name("Gold")) {
		gold() += amnt;
	} else {
//		if (is_same_projectile(equipment().projectile(), type)) {
//			equipment().projectile_slot().add_copies(amnt);
		if (!equipment().has_projectile()
				&& is_wieldable_projectile(equipment(), type)) {
			equip_as_well = true;
		}
		inventory().add(type, equip_as_well);
	}

	cooldowns().reset_pickup_cooldown(PICKUP_RATE);
	gs->remove_instance(iteminst);
}

void PlayerInst::enqueue_network_actions(GameState *gs) {
	GameNetConnection& connection = gs->net_connection();
	bool hasconnection = connection.connection() != NULL;
//TODO: net redo
//	NetPacket packet;
//
//	if (!is_local_player() && hasconnection) {
//		//	printf("Waiting for player action for frame %d\n", gs->frame());
//		while (!connection.get_connection()->get_next_packet(packet)) {
//		}
//		bool output1 = true;
//		while (packet.body_length > 0) {
//			GameAction action;
//			packet.get(action);
//			if (output1 && action.frame != gs->frame()) {
//				fprintf(stderr, "action frame %d vs %d\n", action.frame,
//						gs->frame());
//				fflush(stderr);
//				output1 = false;
//			}
//			queued_actions.push_front(action);
//		}
//	}
}

void PlayerInst::perform_queued_actions(GameState* gs) {
	GameSettings& settings = gs->game_settings();

	if (settings.saving_to_action_file()) {
		save_actions(gs, queued_actions);
	}

	for (int i = 0; i < queued_actions.size(); i++) {
		perform_action(gs, queued_actions[i]);
	}
	queued_actions.clear();
	actions_set_for_turn = false;
}

void PlayerInst::drop_item(GameState* gs, const GameAction& action) {
	ItemSlot& itemslot = inventory().get(action.use_id);
	int dropx = round_to_multiple(x, TILE_SIZE, true), dropy =
			round_to_multiple(y, TILE_SIZE, true);
	int amnt = itemslot.amount();
	bool already_item_here = gs->object_radius_test(dropx, dropy,
			ItemInst::RADIUS, item_colfilter);
	if (!already_item_here) {
		gs->add_instance(new ItemInst(itemslot.item, dropx, dropy, id));
		itemslot.clear();
	}
	if (this->local) {
		gs->game_hud().reset_slot_selected();
	}
}

void PlayerInst::purchase_from_store(GameState* gs, const GameAction& action) {
	StoreInst* store = (StoreInst*)gs->get_instance(action.use_id);
	if (!store) {
		return;
	}LANARTS_ASSERT(dynamic_cast<StoreInst*>(gs->get_instance(action.use_id)));
	StoreInventory& inv = store->inventory();
	StoreItemSlot& slot = inv.get(action.use_id2);
	if (gold() >= slot.cost) {
		inventory().add(slot.item);
		gold() -= slot.cost;
		slot.item.clear();
	}
}

void PlayerInst::reposition_item(GameState* gs, const GameAction& action) {
	ItemSlot& itemslot1 = inventory().get(action.use_id);
	ItemSlot& itemslot2 = inventory().get(action.use_id2);

	std::swap(itemslot1, itemslot2);
	gs->game_hud().reset_slot_selected();
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
		return equipment().deequip_type(action.use_id);
	case GameAction::REPOSITION_ITEM:
		return reposition_item(gs, action);
	case GameAction::CHOSE_SPELL:
		spellselect = action.use_id;
		return;
	case GameAction::PURCHASE_FROM_STORE:
		return purchase_from_store(gs, action);
	default:
		printf("PlayerInst::perform_action() error: Invalid action id %d!\n",
				action.act);
		break;
	}
}

static bool item_check_lua_prereq(lua_State* L, ItemEntry& type,
		GameInst* user) {
	if (type.inventory_use_prereq_func().empty())
		return true;

	type.inventory_use_prereq_func().push(L);
	luayaml_push_item(L, type.name.c_str());
	lua_push_gameinst(L, user);
	lua_call(L, 2, 1);

	bool ret = lua_toboolean(L, lua_gettop(L));
	lua_pop(L, 1);

	return ret;
}
static void item_do_lua_action(lua_State* L, ItemEntry& type, GameInst* user,
		const Pos& p, int amnt) {
	type.inventory_use_func().push(L);
	luayaml_push_item(L, type.name.c_str());
	lua_push_gameinst(L, user);
	lua_pushnumber(L, p.x);
	lua_pushnumber(L, p.y);
	lua_pushnumber(L, amnt);
	lua_call(L, 5, 0);
}
void PlayerInst::use_item(GameState* gs, const GameAction& action) {
	if (!effective_stats().allowed_actions.can_use_items) {
		return;
	}
	itemslot_t slot = action.use_id;
	ItemSlot& itemslot = inventory().get(slot);
	Item& item = itemslot.item;
	ItemEntry& type = itemslot.item_entry();

	lua_State* L = gs->get_luastate();

	if (item.amount > 0) {
		if (item.is_equipment()) {
			if (itemslot.is_equipped()) {
				itemslot.deequip();
			} else {
				if (!item.is_projectile()
						|| equipment().valid_to_use_projectile(item)) {
					equipment().equip(slot);
				}
			}
		} else if (equipment().valid_to_use(item)
				&& item_check_lua_prereq(L, type, this)) {
			item_do_lua_action(L, type, this,
					Pos(action.action_x, action.action_y), item.amount);
			if (is_local_player() && !type.inventory_use_message().empty()) {
				gs->game_chat().add_message(type.inventory_use_message(),
						Colour(100, 100, 255));
			}
			if (item.is_projectile())
				itemslot.clear();
			else
				item.remove_copies(1);
			reset_rest_cooldown();
		}
	}
}
void PlayerInst::use_rest(GameState* gs, const GameAction& action) {
	if (!effective_stats().allowed_actions.can_use_rest) {
		return;
	}
	bool atfull = core_stats().hp >= core_stats().max_hp
			&& core_stats().mp >= core_stats().max_mp;
	if (cooldowns().can_rest() && !atfull && effects().can_rest()) {
		core_stats().heal_hp(core_stats().hpregen * 8);
		core_stats().heal_mp(core_stats().mpregen * 8);
		is_resting = true;
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

	Pos newpos(round(rx + ddx), round(ry + ddy));

	if (!gs->tile_radius_test(newpos.x, newpos.y, radius)) {
		vx = ddx;
		vy = ddy;
	} else if (!gs->tile_radius_test(newpos.x, y, radius)) {
		vx = ddx;
	} else if (!gs->tile_radius_test(x, newpos.y, radius)) {
		vy = ddy;
	} else if (ddx != 0 && ddy != 0) {
		//Alternatives in opposite directions for x & y
		Pos newpos_alt1(round(vx + ddx), round(vy - ddy));
		Pos newpos_alt2(round(vx - ddx), round(vy + ddy));
		if (!gs->tile_radius_test(newpos_alt1.x, newpos_alt1.y, radius)) {
			vx += ddx;
			vy -= ddy;
		} else if (!gs->tile_radius_test(newpos_alt2.x, newpos_alt2.y,
				radius)) {
			vx -= ddx;
			vy += ddy;
		}

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
	if (!effective_stats().allowed_actions.can_use_stairs) {
		if (is_local_player()) {
			gs->game_chat().add_message(
					"You cannot use the stairs in this state!");
		}
		return;
	}
	Pos hitpos;
	bool didhit = gs->tile_radius_test(x, y, radius, false,
			get_tile_by_name("stairs_up"), &hitpos);
	int entr_n = scan_entrance(gs->get_level()->exits, hitpos);
	if (!didhit || entr_n == -1)
		return;

	LANARTS_ASSERT( entr_n >= 0 && entr_n < gs->get_level()->exits.size());
	gs->ensure_level_connectivity(gs->get_level()->id() - 1,
			gs->get_level()->id());
	GameLevelPortal* portal = &gs->get_level()->exits[entr_n];

	int px = centered_multiple(portal->exitsqr.x, TILE_SIZE);
	int py = centered_multiple(portal->exitsqr.y, TILE_SIZE);
	gs->level_move(id, px, py, gs->get_level()->id(),
			gs->get_level()->id() - 1);

	reset_rest_cooldown();
}
void PlayerInst::use_dngn_entrance(GameState* gs, const GameAction& action) {
	if (!effective_stats().allowed_actions.can_use_stairs) {
		if (is_local_player()) {
			gs->game_chat().add_message(
					"You cannot use the stairs in this state!");
		}
		return;
	}
	Pos hitpos;
	bool didhit = gs->tile_radius_test(x, y, radius, false,
			get_tile_by_name("stairs_down"), &hitpos);
	int entr_n = scan_entrance(gs->get_level()->entrances, hitpos);
	if (!didhit || entr_n == -1)
		return;

	LANARTS_ASSERT( entr_n >= 0 && entr_n < gs->get_level()->entrances.size());
	gs->ensure_level_connectivity(gs->get_level()->id(),
			gs->get_level()->id() + 1);
	GameLevelPortal* portal = &gs->get_level()->entrances[entr_n];

	int px = centered_multiple(portal->exitsqr.x, TILE_SIZE);
	int py = centered_multiple(portal->exitsqr.y, TILE_SIZE);
	gs->level_move(id, px, py, gs->get_level()->id(),
			gs->get_level()->id() + 1);

	reset_rest_cooldown();
}

void PlayerInst::gain_xp(GameState* gs, int xp) {
	int levels_gained = stats().gain_xp(xp);
	if (levels_gained > 0) {
		char level_gain_str[128];
		snprintf(level_gain_str, 128, "%s reached level %d!",
				is_local_player() ? "You have" : "Your ally has",
				class_stats().xplevel);
		gs->game_chat().add_message(level_gain_str, Colour(50, 205, 50));
	}
}
