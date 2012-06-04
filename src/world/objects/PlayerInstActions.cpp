/*
 * PlayerInstActions.cpp
 *  Implements the various actions that the player can perform, handles keyboard & mouse IO
 *  as well as networking communication of actions
 */

extern "C" {
#include <lua/lua.h>
}

#include "../../data/item_data.h"
#include "../../data/sprite_data.h"
#include "../../data/tile_data.h"
#include "../../data/weapon_data.h"

#include "../../display/display.h"

#include "../../lua/lua_api.h"

#include "../../util/math_util.h"
#include "../../util/collision_util.h"
#include "../../util/game_basic_structs.h"

#include "../GameState.h"

#include "../utility_objects/AnimatedInst.h"

#include "PlayerInst.h"
#include "EnemyInst.h"
#include "ItemInst.h"

#include "ProjectileInst.h"

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

static void get_visible_monsters(GameState* gs,
		std::vector<GameInst*>& visible_monsters, PlayerInst* p = NULL) {
	const std::vector<obj_id>& mids = gs->monster_controller().monster_ids();
	for (int i = 0; i < mids.size(); i++) {
		GameInst* inst = gs->get_instance(mids[i]);
		if (gs->object_visible_test(inst, p)) {
			visible_monsters.push_back(inst);
		}
	}
}

static GameInst* find_closest_from_list(GameState* gs,
		std::vector<GameInst*>& candidates, const Pos& pos, int* dist = NULL) {
	GameInst* closest = NULL;
	int mindist = -1;

	for (int i = 0; i < candidates.size(); i++) {
		GameInst* inst = candidates[i];
		float dx = inst->x - pos.x, dy = inst->y - pos.y;
		float dist = sqrt(dx * dx + dy * dy);
		if (mindist == -1 || dist < mindist) {
			mindist = dist;
			closest = inst;
		}
	}

	if (dist)
		*dist = mindist;

	return closest;
}
static bool find_blink_target(PlayerInst* p, GameState* gs, Pos& position) {
	PlayerController& pc = gs->player_controller();

	std::vector<GameInst*> visible_monsters;
	get_visible_monsters(gs, visible_monsters);

	int maxdist = 0;
	for (int i = 0; i < pc.player_ids().size(); i++) {

		fov* fov = pc.player_fovs()[i];
		BBox fbox = fov->tiles_covered();
		FOR_EACH_BBOX(fbox, x, y) {
			if (fov->within_fov(x, y)) {
				Pos pos(x * TILE_SIZE + TILE_SIZE / 2,
						y * TILE_SIZE + TILE_SIZE / 2);

				if (gs->solid_test(p, pos.x, pos.y))
					continue;

				int mindist;
				find_closest_from_list(gs, visible_monsters, pos, &mindist);

				if (mindist > TILE_SIZE / 2 && mindist > maxdist) {
					maxdist = mindist;
					position = pos;
				}
			}
		}
	}
	if (maxdist != 0) {
		float dx = p->x - position.x, dy = p->y - position.y;
		float dist = sqrt(dx * dx + dy * dy);
		return (dist > TILE_SIZE / 2);
	} else
		return false;
}

void PlayerInst::queue_io_spell_actions(GameState* gs) {
	GameView& view = gs->window_view();
	bool mouse_within = gs->mouse_x() < gs->window_view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	int level = gs->level()->roomid, frame = gs->frame();
	bool spell_used = false;

	//Keyboard-oriented blink
	if (!spell_used && gs->key_press_state(SDLK_h)) {
		Pos blinkposition;
		if (stats().mp >= 50 && find_blink_target(this, gs, blinkposition)) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_SPELL, frame, level, 2,
							blinkposition.x, blinkposition.y));
			spell_used = true;
		}
	}

	//Spell use
	if (!spell_used && gs->key_down_state(SDLK_j)) {
		MonsterController& mc = gs->monster_controller();
		GameInst* target = gs->get_instance(mc.current_target());
		int mpcost = 10;
		if (spellselect)
			mpcost = 20;
		if (target && (spellselect == -1 || stats().mp < mpcost)) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_WEAPON, frame, level,
							spellselect, target->x, target->y));
			spell_used = true;
		} else {
			if (target && !stats().has_cooldown() && stats().mp >= mpcost
					&& gs->object_visible_test(target, this)) {
				queued_actions.push_back(
						GameAction(id, GameAction::USE_SPELL, frame, level,
								spellselect, target->x, target->y));
				spell_used = true;
			}
		}
	}
	if (!spell_used && gs->mouse_right_click() && mouse_within) {

		int px = x, py = y;
		x = rmx, y = rmy;
		if (stats().mp >= 50 && !gs->solid_test(this)
				&& gs->object_visible_test(this)) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_SPELL, frame, level, 2, x,
							y));
		}
		x = px, y = py;
	}

	if (!spell_used && gs->mouse_left_down() && mouse_within) {
		int mpcost = 10;
		if (spellselect)
			mpcost = 20;
		if (spellselect == -1 || stats().mp < mpcost) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_WEAPON, frame, level,
							spellselect, rmx, rmy));
			spell_used = true;
		} else if (!stats().has_cooldown() && stats().mp >= mpcost) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_SPELL, frame, level,
							spellselect, rmx, rmy));
			spell_used = true;
		}
	}
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
	const int ITEM_PICKUP_RATE = 10;
	if (frame % ITEM_PICKUP_RATE == 0
			&& gs->object_radius_test(this, &item, 1, &item_colfilter)) {
		ItemInst* iteminst = (ItemInst*) item;
		int type = iteminst->item_type();
		bool autopickup = game_item_data[type].equipment_type == ItemEntry::NONE
				&& iteminst->last_held_by() != id;
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
		if (gs->key_down_state(SDLK_r) && canrestcooldown == 0) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_REST, frame, level));
			resting = true;
		}

		if (!resting) {
			queue_io_spell_actions(gs);
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
	ItemInst* item = (ItemInst*) gs->get_instance(action.use_id);
	if (!item)
		return;
	if (item->item_type() == get_item_by_name("Gold")) {
		money += 10;
	} else {
		get_inventory().add(item->item_type(), item->item_quantity());
	}
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
	int gx = x / TILE_SIZE, gy = y / TILE_SIZE;
	int dropx = gx * TILE_SIZE + TILE_SIZE / 2, dropy = gy * TILE_SIZE
			+ TILE_SIZE / 2;
	gs->add_instance(new ItemInst(itemslot.item, dropx, dropy, 1, id));
	itemslot.amount--;
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

	if (itemslot.amount > 0 && item_check_lua_prereq(L, type, this->id)) {
		item_do_lua_action(L, type, this->id,
				Pos(action.action_x, action.action_y), itemslot.amount);
		if (type.equipment_type == ItemEntry::PROJECTILE)
			itemslot.amount = 0;
		else
			itemslot.amount--;
		canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
	}

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

	float mag = effective_stats(gs->get_luastate()).movespeed;

	float ddx = dx * mag;
	float ddy = dy * mag;

	EnemyInst* target = NULL;
	//Enemy hitting test for melee
	gs->object_radius_test(this, (GameInst**) &target, 1, &enemy_colfilter,
			x + ddx * 2, y + ddy * 2);

	//Smaller radius enemy pushing test, can intercept enemy radius but not too far
	EnemyInst* alreadyhitting[5] = { 0, 0, 0, 0, 0 };
	gs->object_radius_test(this, (GameInst**) alreadyhitting, 5,
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
//
	int px = (portal->exitsqr.x) * TILE_SIZE + TILE_SIZE / 2;
	int py = (portal->exitsqr.y) * TILE_SIZE + TILE_SIZE / 2;
	gs->level_move(id, px, py, gs->level()->roomid, gs->level()->roomid - 1);

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
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

	int px = (portal->exitsqr.x) * TILE_SIZE + TILE_SIZE / 2;
	int py = (portal->exitsqr.y) * TILE_SIZE + TILE_SIZE / 2;
	gs->level_move(id, px, py, gs->level()->roomid, gs->level()->roomid + 1);

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
}

void PlayerInst::use_spell(GameState* gs, const GameAction& action) {
	Stats estats = effective_stats(gs->get_luastate());
	if (action.use_id < 2 && stats().has_cooldown())
		return;

	if (action.use_id == 0) {
		stats().mp -= 10;
	} else if (action.use_id == 1) {
		stats().mp -= 20;
	} else if (action.use_id == 2) {
		stats().mp -= 50;
	}

	Attack atk(estats.magicatk);
	bool bounce = true;
	int hits = 0;

	if (action.use_id == 1) {
		atk.attack_sprite = get_sprite_by_name("magic blast");
		atk.projectile_speed /= 1.75;
		//	atk.damage *= 2;
		bounce = false;
		hits = 3;
	}
	atk.damage = estats.calculate_spell_damage(gs->rng(), action.use_id);

	if (action.use_id < 2) {
		GameInst* bullet = new ProjectileInst(atk.attack_sprite, id,
				atk.projectile_speed, atk.range, atk.damage, x, y,
				action.action_x, action.action_y, bounce, hits);
		gs->add_instance(bullet);
	} else {
		x = action.action_x;
		y = action.action_y;
	}

	if (action.use_id == 1)
		base_stats.cooldown = estats.magicatk.cooldown * 1.4;
	else if (action.use_id == 0) {
		double mult = 1 + base_stats.xplevel / 8.0;
		mult = std::min(2.0, mult);
		base_stats.cooldown = estats.magicatk.cooldown / mult;
	} else if (action.use_id == 2) {
		base_stats.cooldown = estats.magicatk.cooldown * 2;
	}

	canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
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

static int get_targets(GameState* gs, PlayerInst* p, int ax, int ay, int rad, GameInst** enemies, int max_targets){
	int numhit = gs->object_radius_test(p, enemies, max_targets,
			enemy_colfilter, ax, ay, rad);
	if (numhit < max_targets){
		std::vector<GameInst*> visible_monsters;
		get_visible_monsters(gs, visible_monsters, p);

		GameInst* inst = find_closest_from_list(gs, visible_monsters, Pos(p->x, p->y));

		numhit += gs->object_radius_test(p, enemies + numhit, max_targets - numhit,
					enemy_colfilter, inst->x, inst->y, rad);
	}
	return numhit;
}

void PlayerInst::use_weapon(GameState *gs, const GameAction& action) {
	const int MAX_MELEE_HITS = 10;
	Stats estats = effective_stats(gs->get_luastate());
	if (estats.has_cooldown())
		return;

	WeaponEntry& weap = game_weapon_data[weapon_type()];

	int dx = action.action_x - x, dy = action.action_y - y;
	float mag = sqrt(dx * dx + dy * dy);
	if (mag == 0)
		mag = 1;
	int ax = x + weap.range * dx / mag, ay = y + weap.range * dy / mag;

	GameInst* enemies[MAX_MELEE_HITS];

	int max_targets = std::min(MAX_MELEE_HITS, weap.max_targets);

	int numhit = get_targets(gs, this, ax, ay, weap.dmgradius, enemies, max_targets);

	for (int i = 0; i < numhit; i++) {
		EnemyInst* e = (EnemyInst*) enemies[i];
		//int damage = effective_stats().melee.damage + gs->rng().rand(-4, 5);
		WeaponEntry& wtype = game_weapon_data[weapon_type()];
		int damage = estats.calculate_melee_damage(gs->rng(), weapon_type());
		char buffstr[32];
		snprintf(buffstr, 32, "%d", damage);
		float rx, ry;
		direction_towards(Pos(x, y), Pos(e->x, e->y), rx, ry, 0.5);
		gs->add_instance(
				new AnimatedInst(e->x - 5 + rx * 5, e->y - 3 + rx * 5, -1, 25,
						rx, ry, buffstr, Colour(255, 148, 120)));

		if (e->hurt(gs, damage)) {
			gain_xp(gs, e->xpworth());
			if (is_local_player()) {
				snprintf(buffstr, 32, "%d XP", e->xpworth());
				gs->add_instance(
						new AnimatedInst(e->x - 5, e->y - 5, -1, 25, 0, 0,
								buffstr, Colour(255, 215, 11)));
			}
		}
		stats().cooldown = wtype.cooldown;
		//stats().reset_melee_cooldown(effective_stats());
		int atksprite = game_weapon_data[weapon_type()].attack_sprite;
		gs->add_instance(new AnimatedInst(e->x, e->y, atksprite, 25));

	}
}
