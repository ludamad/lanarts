/*
 * PlayerInstActions.cpp
 *  Implements the various actions that the player can perform, handles keyboard & mouse IO
 *  as well as networking communication of actions
 */

#include <lua.hpp>

#include <luawrap/luawrap.h>

#include <lcommon/strformat.h>

#include "draw/draw_sprite.h"
#include "draw/SpriteEntry.h"
#include "draw/TileEntry.h"

#include "draw/colour_constants.h"

#include "gamestate/GameState.h"
#include "lua_api/lua_api.h"

#include "stats/items/ItemEntry.h"

#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"

#include "util/game_replays.h"
#include <lcommon/math_util.h>

#include "lanarts_defines.h"

#include "../enemy/EnemyInst.h"

#include "../store/StoreInst.h"

#include "../AnimatedInst.h"
#include "../FeatureInst.h"
#include "../ItemInst.h"
#include "../ProjectileInst.h"
#include "../collision_filters.h"

#include "PlayerInst.h"

static FeatureInst* find_usable_portal(GameState* gs, GameInst* player) {
	std::vector<GameInst*> instances = gs->get_level()->game_inst_set().to_vector();
	for (int i = 0; i < instances.size(); i++) {
		FeatureInst* inst = dynamic_cast<FeatureInst*>(instances[i]);
		if (inst == NULL) {
			continue;
		}
		if (inst->feature_type() == FeatureInst::PORTAL) {
			Pos sqr = Pos(inst->x / TILE_SIZE, inst->y /TILE_SIZE);
			BBox portal_box(Pos(sqr.x*TILE_SIZE, sqr.y*TILE_SIZE), Size(TILE_SIZE, TILE_SIZE));
			bool hit_test = circle_rectangle_test(player->ipos(), player->target_radius, portal_box);
			if (hit_test) {
				return inst;
			}
		}
	}
	return NULL;
}

static bool queue_portal_use(GameState* gs, PlayerInst* player, ActionQueue& queue) {
	FeatureInst* portal = find_usable_portal(gs, player);
	if (portal != NULL) {
		queue.push_back(
				game_action(gs, player, GameAction::USE_PORTAL));
		return true;
	}
	return false;
}


// Determines if a projectile should be wielded from the ground
static bool projectile_should_autowield(EquipmentStats& equipment,
		const Item& item, const std::string& preferred_class) {
	if (!item.is_projectile())
		return false;

	Projectile equipped_projectile = equipment.projectile();
	if (!equipped_projectile.empty()) {
		return equipped_projectile.is_same_item(item);
	}

	ProjectileEntry& pentry = item.projectile_entry();

	if (pentry.weapon_class == "unarmed") {
		return false;
	}

	if (pentry.weapon_class == preferred_class) {
		if (projectile_compatible_weapon(equipment.inventory, item) != -1) {
			return true;
		}
	}

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

		bool wieldable_projectile = projectile_should_autowield(equipment(),
				item, this->last_chosen_weaponclass);

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

	bool single_player = (gs->player_data().all_players().size() <= 1);

	actions_set_for_turn = true;

	GameSettings& settings = gs->game_settings();
	GameView& view = gs->view();

	PlayerDataEntry& pde = gs->player_data().local_player_data();

	if (pde.action_queue.has_actions_for_frame(gs->frame())) {
		pde.action_queue.extract_actions_for_frame(queued_actions, gs->frame());
		event_log("Player %d has %d actions", player_entry(gs).net_id,
				(int) queued_actions.size());
		return;
	}
	if (!single_player) {
		gs->set_repeat_actions_counter(settings.frame_action_repeat);
	}

	int dx = 0, dy = 0;
	bool mouse_within = gs->mouse_x() < gs->view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	bool was_moving = moving, do_stopaction = false;
	IOController& io = gs->io_controller();

	if (!settings.loadreplay_file.empty()) {
		load_actions(gs, queued_actions);
	}

	enqueue_io_movement_actions(gs, dx, dy);

	if (was_moving && !moving && cooldowns().can_do_stopaction()) {
		do_stopaction = true;
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
		queue_portal_use(gs, this, queued_actions);
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

	int repeat = single_player ? 0 : settings.frame_action_repeat;
	for (int i = 1; i <= repeat; i++) {
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

	bool inventory_full = false;
	if (type.id == get_item_by_name("Gold")) {
		gold() += amnt;
	} else {
		itemslot_t slot = inventory().add(type);
		if (slot == -1) {
			inventory_full = true;
		} else if (projectile_should_autowield(equipment(), type,
				this->last_chosen_weaponclass)) {
			projectile_smart_equip(inventory(), slot);
		}
	}

	if (!inventory_full) {
		cooldowns().reset_pickup_cooldown(PICKUP_RATE);
		gs->remove_instance(iteminst);
	}
}

void PlayerInst::perform_queued_actions(GameState* gs) {
	perf_timer_begin(FUNCNAME);
	GameSettings& settings = gs->game_settings();

	if (settings.saving_to_action_file()) {
		save_actions(gs, queued_actions);
	}

	for (int i = 0; i < queued_actions.size(); i++) {
		perform_action(gs, queued_actions[i]);
	}
	queued_actions.clear();
	actions_set_for_turn = false;
	perf_timer_end(FUNCNAME);
}

void PlayerInst::drop_item(GameState* gs, const GameAction& action) {
	ItemSlot& itemslot = inventory().get(action.use_id);
	int dropx = round_to_multiple(x, TILE_SIZE, true), dropy =
			round_to_multiple(y, TILE_SIZE, true);
	int amnt = itemslot.amount();
	bool already_item_here = gs->object_radius_test(dropx, dropy,
			ItemInst::RADIUS, item_colfilter);
	if (!already_item_here) {
		gs->add_instance(new ItemInst(itemslot.item, Pos(dropx, dropy), id));
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
	}
	LANARTS_ASSERT(dynamic_cast<StoreInst*>(gs->get_instance(action.use_id)));
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
	event_log("Player id=%d performing act=%d, xy=(%d,%d), frame=%d, origin=%d, room=%d, use_id=%d, use_id2=%d\n",
			this->player_entry(gs).net_id,
			action.act, action.action_x,
			action.action_y, action.frame, action.origin, action.room,
			action.use_id, action.use_id2);
	switch (action.act) {
	case GameAction::MOVE:
		return use_move(gs, action);
	case GameAction::USE_WEAPON:
		return use_weapon(gs, action);
	case GameAction::USE_SPELL:
		return use_spell(gs, action);
	case GameAction::USE_REST:
		return use_rest(gs, action);
	case GameAction::USE_PORTAL:
		return use_dngn_portal(gs, action);
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
	LuaValue& prereq = type.inventory_use_prereq_func().get(L);
	if (prereq.empty())
		return true;

	prereq.push();
	luayaml_push_item(L, type.name.c_str());
	luawrap::push(L, user);
	lua_call(L, 2, 1);

	bool ret = lua_toboolean(L, lua_gettop(L));
	lua_pop(L, 1);

	return ret;
}
static void item_do_lua_action(lua_State* L, ItemEntry& type, GameInst* user,
		const Pos& p, int amnt) {
	LuaValue& usefunc = type.inventory_use_func().get(L);
	usefunc.push();
	luayaml_push_item(L, type.name.c_str());
	luawrap::push(L, user);
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

	lua_State* L = gs->luastate();

	if (item.amount > 0) {
		if (item.is_equipment()) {
			if (itemslot.is_equipped()) {
				inventory().deequip(slot);
			} else {
				if (item.is_projectile()) {
					// Best-effort to equip, may not be possible:
					projectile_smart_equip(inventory(), slot);
				} else if (item.is_weapon()) {
					const Projectile& p = equipment().projectile();
					if (!p.empty()) {
						if (!p.projectile_entry().is_standalone()) {
							inventory().deequip_type(
									EquipmentEntry::AMMO);
						}
					}
					equipment().equip(slot);
					// Try and equip a projectile
					WeaponEntry& wentry = item.weapon_entry();
					if (wentry.uses_projectile) {
						const Projectile& p = equipment().projectile();
						if (p.empty()) {
							projectile_smart_equip(inventory(),
									wentry.weapon_class);
						}
					}
				} else {
					equipment().equip(slot);
				}

				if (item.is_weapon() || item.is_projectile()) {
					last_chosen_weaponclass =
							weapon().weapon_entry().weapon_class;
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
	CoreStats& ecore = effective_stats().core;
	int emax_hp = ecore.max_hp, emax_mp = ecore.max_mp;
	bool atfull = core_stats().hp >= emax_hp && core_stats().mp >= emax_mp;
	if (cooldowns().can_rest() && !atfull && effects().can_rest()) {
		core_stats().heal_hp(ecore.hpregen * 8, emax_hp);
		core_stats().heal_mp(ecore.mpregen * 8, emax_mp);
		ecore.hp = core_stats().hp;
		ecore.mp = core_stats().mp;
		is_resting = true;
	}
}
void PlayerInst::use_move(GameState* gs, const GameAction& action) {
	perf_timer_begin(FUNCNAME);
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

	event_log("Player id: %d using move for turn %d, vx=%f, vy=%f\n", id, gs->frame(), vx, vy);
	perf_timer_end(FUNCNAME);
}

void PlayerInst::use_dngn_portal(GameState* gs, const GameAction& action) {
	if (!effective_stats().allowed_actions.can_use_stairs) {
		if (is_local_player()) {
			gs->game_chat().add_message(
					"You cannot use the exit in this state!");
		}
		return;
	}

	cooldowns().reset_stopaction_timeout(50);
	FeatureInst* portal = find_usable_portal(gs, this);
	portal->player_interact(gs, this);
	reset_rest_cooldown();

	std::string subject_and_verb = "You travel";
	if (!is_local_player()) {
		subject_and_verb = player_entry(gs).player_name + " travels";
	}

	const std::string& map_label =
			gs->game_world().get_level(current_floor)->label();

	bool label_has_digit = false; // Does it have a number in the label?
	for (int i = 0; i < map_label.size(); i++) {
		if (isdigit(map_label[i])) {
			label_has_digit = true;
			break;
		}
	}

	gs->game_chat().add_message(
			format("%s to %s%s", subject_and_verb.c_str(),
					label_has_digit ? "" : "the ", map_label.c_str()),
			is_local_player() ? COL_WHITE : COL_YELLOW);
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
