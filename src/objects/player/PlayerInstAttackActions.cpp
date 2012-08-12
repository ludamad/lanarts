/*
 * PlayerInstAttackActions.cpp
 *  Implements the various attack and misc spell actions the player can perform.
 */

extern "C" {
#include <lua/lua.h>
}

#include "../../stats/item_data.h"
#include "../../stats/spell_data.h"
#include "../../display/sprite_data.h"
#include "../../display/tile_data.h"
#include "../../stats/projectile_data.h"
#include "../../stats/weapon_data.h"

#include "../../display/display.h"

#include "../../lua/lua_api.h"

#include "../../display/colour_constants.h"
#include "../../util/math_util.h"
#include "../../lanarts_defines.h"
#include "../collision_filters.h"

#include "../../gamestate/GameState.h"

#include "../AnimatedInst.h"

#include "PlayerInst.h"
#include "../enemy/EnemyInst.h"
#include "../ItemInst.h"
#include "../ProjectileInst.h"

static void get_visible_monsters(GameState* gs,
		std::vector<GameInst*>& visible_monsters, PlayerInst* p = NULL) {
	const std::vector<obj_id>& mids = gs->monster_controller().monster_ids();
	for (int i = 0; i < mids.size(); i++) {
		GameInst* inst = gs->get_instance(mids[i]);
		if (inst && gs->object_visible_test(inst, p)) {
			visible_monsters.push_back(inst);
		}
	}
}

static GameInst* find_closest_from_list(GameState* gs,
		std::vector<GameInst*>& candidates, const Pos& pos,
		float* dist = NULL) {
	GameInst* closest = NULL;
	float mindist = -1;

	for (int i = 0; i < candidates.size(); i++) {
		GameInst* inst = candidates[i];
		float dx = inst->x - pos.x, dy = inst->y - pos.y;
		float dist = sqrt(dx * dx + dy * dy) - inst->target_radius;
		if (closest == NULL || dist < mindist) {
			mindist = dist;
			closest = inst;
		}
	}

	if (dist) {
		*dist = mindist;
	}

	return closest;
}

static GameInst* get_closest_monster(GameState* gs, PlayerInst* p) {
	std::vector<GameInst*> visible_monsters;
	get_visible_monsters(gs, visible_monsters, p);

	return find_closest_from_list(gs, visible_monsters, p->pos());
}
static GameInst* get_weapon_autotarget(GameState* gs, PlayerInst* p,
		GameInst* targ, float dx, float dy) {
	WeaponEntry& wentry = p->weapon().weapon_entry();
	Pos ppos(p->x, p->y);
	GameInst* inst = NULL;
	bool ismelee = !(wentry.uses_projectile || p->equipment().has_projectile());
	int target_range = wentry.range + p->target_radius;

	if (targ
			&& distance_between(Pos(targ->x, targ->y), ppos)
					- targ->target_radius <= target_range) {
		return targ;
	}

	if (ismelee) {
		/*normalize dx & dy*/
		float mag = sqrt(dx * dx + dy * dy);
		if (mag == 0.0f)
			mag = 1.0f;
		dx *= target_range / mag, dy *= target_range / mag;

		/*test in our walking direction first*/
		gs->object_radius_test(p, &inst, 1, enemy_colfilter, p->x + dx,
				p->y + dy, p->radius);
	} else {
		target_range *= 1.25;
	}

	if (inst) {
		return inst;
	} else {
		std::vector<GameInst*> visible_monsters;
		get_visible_monsters(gs, visible_monsters, p);

		float dist;
		GameInst* inst = find_closest_from_list(gs, visible_monsters, ppos,
				&dist);
		if (inst && (!ismelee || dist <= target_range)) {
			return inst;
		}
	}
	return NULL;
}
bool find_safest_square(PlayerInst* p, GameState* gs, Pos& position) {
	PlayerData& pc = gs->player_data();

	std::vector<PlayerInst*> players = gs->players_in_level();
	std::vector<GameInst*> visible_monsters;
	get_visible_monsters(gs, visible_monsters);

	int maxdist = 0;
	for (int i = 0; i < players.size(); i++) {
		BBox fbox = players[i]->field_of_view().tiles_covered();
		FOR_EACH_BBOX(fbox, x, y) {
			if (players[i]->field_of_view().within_fov(x, y)) {
				Pos pos(x * TILE_SIZE + TILE_SIZE / 2,
						y * TILE_SIZE + TILE_SIZE / 2);

				if (gs->solid_test(p, pos.x, pos.y))
					continue;

				float mindist;
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
	} else {
		return false;
	}
}

static int get_targets(GameState* gs, PlayerInst* p, int ax, int ay, int rad,
		GameInst** enemies, int max_targets) {
	int numhit = gs->object_radius_test(p, enemies, max_targets,
			enemy_colfilter, ax, ay, rad);
	if (numhit < max_targets) {
		std::vector<GameInst*> visible_monsters;
		get_visible_monsters(gs, visible_monsters, p);

		GameInst* inst = find_closest_from_list(gs, visible_monsters,
				Pos(p->x, p->y));

		if (inst) {
			numhit += gs->object_radius_test(p, enemies + numhit,
					max_targets - numhit, enemy_colfilter, inst->x, inst->y,
					rad);
		}
	}
	return numhit;
}

static bool lua_spell_get_target(GameState* gs, PlayerInst* p, LuaValue& action,
		Pos& pos) {
	bool nilresult = false;
	lua_State* L = gs->get_luastate();
	obj_id target_id = gs->monster_controller().current_target();
	GameInst* target = gs->get_instance(target_id);

	int beforecall_idx = lua_gettop(L);
	action.push(L);
	lua_push_gameinst(L, p);
	if (!target) {
		lua_pushnil(L);
	} else {
		lua_push_gameinst(L, target);
	}

	// Allow for multiple return values
	// read the stack size difference to find out how many were returned
	lua_call(L, 2, LUA_MULTRET);
	int nret = lua_gettop(L) - beforecall_idx;

	if (nret != 2 || lua_isnil(L, -1)) {
		nilresult = true;
	} else if (nret == 2) {
		pos.x = lua_tointeger(L, -2);
		pos.y = lua_tointeger(L, -1);
	}

	lua_pop(L, nret);

	return !nilresult;
}

static void player_use_luacallback_spell(GameState* gs, PlayerInst* p,
		SpellEntry& spl_entry, LuaValue& action, const Pos& target) {
	lua_State* L = gs->get_luastate();
	action.push(L);
	lua_push_gameinst(L, p);
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);
	lua_call(L, 3, 0);
}

static bool lua_spell_check_prereq(GameState* gs, PlayerInst* p,
		SpellEntry& spl_entry, LuaValue& action, const Pos& target) {
	lua_State* L = gs->get_luastate();
	bool passes = true;

	action.push(L);
	if (!lua_isnil(L, -1)) {
		lua_push_gameinst(L, p);
		lua_pushnumber(L, target.x);
		lua_pushnumber(L, target.y);
		lua_call(L, 3, 1);
		passes = lua_toboolean(L, -1);
	}

	/*Pop either the nill or the boolean*/
	lua_pop(L, 1);
	return passes;
}

static void player_use_projectile_spell(GameState* gs, PlayerInst* p,
		SpellEntry& spl_entry, const Projectile& projectile,
		const Pos& target) {
	MTwist& mt = gs->rng();
	AttackStats projectile_attack(Weapon(), projectile);
	ProjectileEntry& pentry = projectile.projectile_entry();
	bool wallbounce = pentry.can_wall_bounce;
	int nbounces = pentry.number_of_target_bounces;

	GameInst* pinst = new ProjectileInst(projectile,
			p->effective_atk_stats(mt, projectile_attack), p->id,
			Pos(p->x, p->y), target, pentry.speed, pentry.range, NONE,
			wallbounce, nbounces);
	gs->add_instance(pinst);
}

static void player_use_spell(GameState* gs, PlayerInst* p,
		SpellEntry& spl_entry, const Pos& target) {
	p->core_stats().mp -= spl_entry.mp_cost;
	p->cooldowns().reset_action_cooldown(spl_entry.cooldown);
	if (spl_entry.uses_projectile()) {
		player_use_projectile_spell(gs, p, spl_entry, spl_entry.projectile,
				target);
	} else {
		player_use_luacallback_spell(gs, p, spl_entry, spl_entry.action_func,
				target);
	}
}

void PlayerInst::queue_not_enough_mana_actions(GameState* gs) {
	const int AUTOUSE_MANA_POTION_CNT = 2;
	int item_slot = inventory().find_slot(get_item_by_name("Mana Potion"));
	if (gs->game_settings().autouse_mana_potions
			&& autouse_mana_potion_try_count >= AUTOUSE_MANA_POTION_CNT
			&& item_slot != -1) {
		queued_actions.push_back(
				game_action(gs, this, GameAction::USE_ITEM, item_slot));
		autouse_mana_potion_try_count = 0;
	} else {
		autouse_mana_potion_try_count++;
	}
}
bool PlayerInst::queue_io_spell_actions(GameState* gs) {
	GameView& view = gs->view();
	IOController& io = gs->io_controller();
	SpellsKnown& spells = spells_known();

	bool perform_spell = false;
	bool chose_spell = false;
	bool triggered_already = false;

	int newspell = spell_selected();
	//Spell choice
	for (int i = 0; i < spells.amount(); i++) {
		IOEvent event(IOEvent::ACTIVATE_SPELL_N, i);
		if (io.query_event(event, &triggered_already)) {
			chose_spell = true;
			// Use the remembered choice to determine if its appropriate to cast this spell
			// This makes sure the key must be hit once to switch spell, and again to use it
			if (spell_selected() == i
					&& previous_spellselect == spell_selected()) {
				//Double hit a spell switch to quick-perform it
				perform_spell = true;
			} else if (!triggered_already) {
				newspell = i;
			}
			break;
		}
	}
	if (!chose_spell) {
		// If we did not switch a spell with one of the quick-select keys, remember our choice
		previous_spellselect = spell_selected();

		if (io.query_event(IOEvent::TOGGLE_ACTION_UP)) {
			newspell = (spell_selected() + 1) % spells.amount();
		} else if (io.query_event(IOEvent::TOGGLE_ACTION_DOWN)) {
			if (spell_selected() <= 0) {
				newspell = spells.amount() - 1;
			} else {
				newspell = spell_selected() - 1;
			}
		}
	}

	if (newspell != spell_selected()) {
		queued_actions.push_back(
				game_action(gs, this, GameAction::CHOSE_SPELL, newspell));
	}

	bool auto_target = true;
	// We don't auto-target unless a mouse is not used
	if (!perform_spell
			&& io.query_event(IOEvent::MOUSETARGET_CURRENT_ACTION,
					&triggered_already)) {
		perform_spell = true;
		auto_target = false;

	} else if (!perform_spell) {
		perform_spell = io.query_event(IOEvent::AUTOTARGET_CURRENT_ACTION,
				&triggered_already);
	}
	if (spell_selected() > -1 && perform_spell) {
		SpellEntry& spl_entry = spells.get_entry(spell_selected());

		Pos target;
		bool can_trigger = !triggered_already
				|| spl_entry.can_cast_with_held_key;
		bool can_target;
		if (auto_target) {
			can_target = lua_spell_get_target(gs, this,
					spl_entry.autotarget_func, target);
		} else {
			int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();
			target = Pos(rmx, rmy);
			can_target = true;
		}

		if (spl_entry.mp_cost > core_stats().mp) {
			if (!triggered_already && can_target) {
				queue_not_enough_mana_actions(gs);
			}
			return false;
		} else {
			autouse_mana_potion_try_count = 0;
		}

		if (can_trigger && can_target) {
			bool can_use = lua_spell_check_prereq(gs, this, spl_entry,
					spl_entry.prereq_func, target);
			if (can_use) {
				queued_actions.push_back(
						game_action(gs, this, GameAction::USE_SPELL,
								spell_selected(), target.x, target.y));
				return true;
			} else if (!auto_target) {
				gs->game_chat().add_message("Target location is not valid.");
			}
		} else if (!triggered_already && !can_target) {
			gs->game_chat().add_message(
					"Cannot currently auto-target spell. Use manual controls (with mouse).");
		}
	}
	return false;
}

static bool decide_attack_movement(const Pos& player, const Pos& target,
		float threshold, int& vx, int& vy) {
	float dx = target.x - player.x, dy = target.y - player.y;
	if (fabs(dx) < threshold) {
		dx = 0;
	}
	if (fabs(dy) < threshold) {
		dy = 0;
	}
	normalize(dx, dy);
	vx = 0, vy = 0;
	if (dx > 0.2) {
		vx = 1;
	} else if (dx < -0.2) {
		vx = -1;
	}
	if (dy > 0.2) {
		vy = 1;
	} else if (dy < -0.2) {
		vy = -1;
	}
	return vx != 0 || vy != 0;
}

// dx & dy indicates moving direction, useful for choosing melee attack targets
bool PlayerInst::queue_io_spell_and_attack_actions(GameState* gs, float dx,
		float dy) {
	GameView& view = gs->view();
	WeaponEntry& wentry = weapon().weapon_entry();

	bool mouse_within = gs->mouse_x() < gs->view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	int level = gs->get_level()->levelid, frame = gs->frame();

	bool is_moving = (dx != 0.0f || dy != 0.0f);
	IOController& io = gs->io_controller();
	bool attack_used = queue_io_spell_actions(gs);

	bool autotarget = io.query_event(IOEvent::AUTOTARGET_CURRENT_ACTION)
			|| io.query_event(IOEvent::ACTIVATE_SPELL_N);
	bool mousetarget = io.query_event(IOEvent::MOUSETARGET_CURRENT_ACTION);

	bool weaponuse = spell_selected() == -1;

	// choose & use weapon
	if (io.query_event(IOEvent::USE_WEAPON)) {
		queued_actions.push_back(
				game_action(gs, this, GameAction::CHOSE_SPELL, -1));
		autotarget = true;
		weaponuse = true;
	}

	if (spell_selected() >= 0
			&& spells_known().get_entry(spell_selected()).mp_cost
					> core_stats().mp) {
		weaponuse = true;
	}

	// weapon use
	if (!attack_used && weaponuse && (autotarget || mousetarget)) {

		bool is_projectile = wentry.uses_projectile
				|| equipment().has_projectile();

		MonsterController& mc = gs->monster_controller();
		GameInst* curr_target = gs->get_instance(mc.current_target());
		GameInst* target = NULL;
		Pos targ_pos;

		if (is_projectile) {
			if (mousetarget) {
				targ_pos = Pos(rmx, rmy);
			} else if (autotarget && curr_target) {
				targ_pos = curr_target->pos();
			}
		} else {
			if (mousetarget) {
				dx = rmx - x, dy = rmy - y;
			}
			target = get_weapon_autotarget(gs, this, curr_target, dx, dy);
			if (target) {
				targ_pos = Pos(target->x, target->y);

			}
			if (!is_moving && !target && !mousetarget && spell_selected() == -1
					&& curr_target && !is_projectile) {
				int vx, vy;
				GameInst* closest = get_closest_monster(gs, this);

				if (closest
						&& decide_attack_movement(pos(), closest->pos(),
								TILE_SIZE / 4, vx, vy)) {
					queued_actions.push_back(
							game_action(gs, this, GameAction::MOVE, spellselect,
									round(vx), round(vy)));
				}
			}
		}
		if (target || (is_projectile && (mousetarget || curr_target))) {
			queued_actions.push_back(
					game_action(gs, this, GameAction::USE_WEAPON, spellselect,
							targ_pos.x, targ_pos.y));
			attack_used = true;
		}
	}
	return attack_used;
}

static void lua_hit_callback(lua_State* L, LuaValue& callback, GameInst* user,
		GameInst* target) {
	callback.push(L);
	if (!lua_isnil(L, -1)) {
		lua_push_gameinst(L, user);
		lua_push_gameinst(L, target);
		lua_call(L, 2, 0);
	} else {
		lua_pop(L, 1);
	}
}

void PlayerInst::use_weapon(GameState* gs, const GameAction& action) {
	WeaponEntry& wentry = weapon().weapon_entry();
	MTwist& mt = gs->rng();
	const int MAX_MELEE_HITS = 10;
	EffectiveStats& estats = effective_stats();
	if (!cooldowns().can_doaction()) {
		return;
	}

	Pos start(x, y);
	Pos actpos(action.action_x, action.action_y);

	if (wentry.uses_projectile && !equipment().has_projectile()) {
		return;
	}

	if (equipment().has_projectile()) {
		const Projectile& projectile = equipment().projectile;
		ProjectileEntry& pentry = projectile.projectile_entry();
		item_id item = get_item_by_name(pentry.name.c_str());
		int weaprange = std::max(wentry.range, pentry.range);

		AttackStats weaponattack(weapon());
		GameInst* bullet = new ProjectileInst(projectile,
				effective_atk_stats(mt, weaponattack), id, start, actpos,
				pentry.speed, weaprange);
		gs->add_instance(bullet);
		cooldowns().reset_action_cooldown(
				std::max(wentry.cooldown, pentry.cooldown));

		equipment().use_ammo();
	} else {
		int weaprange = wentry.range + this->radius + TILE_SIZE / 2;
		float mag = distance_between(actpos, Pos(x, y));
		if (mag > weaprange) {
			float dx = actpos.x - x, dy = actpos.y - y;
			actpos = Pos(x + dx / mag * weaprange, y + dy / mag * weaprange);
		}

		GameInst* enemies[MAX_MELEE_HITS];

		int max_targets = std::min(MAX_MELEE_HITS, wentry.max_targets);

		int numhit = get_targets(gs, this, actpos.x, actpos.y, wentry.dmgradius,
				enemies, max_targets);

		if (numhit == 0) {
			return;
		}

		for (int i = 0; i < numhit; i++) {
			EnemyInst* e = (EnemyInst*) enemies[i];
			lua_hit_callback(gs->get_luastate(), wentry.on_hit_func, this, e);
			if (attack(gs, e, AttackStats(equipment().weapon))) {
				PlayerData& pc = gs->player_data();
				signal_killed_enemy();

				char buffstr[32];
				int amnt = round(
						double(e->xpworth()) / pc.all_players().size());
				players_gain_xp(gs, amnt);
				snprintf(buffstr, 32, "%d XP", amnt);
				gs->add_instance(
						new AnimatedInst(e->x - 5, e->y - 5, -1, 25, 0, 0,
								AnimatedInst::DEPTH, buffstr,
								Colour(255, 215, 11)));
			}
		}
		cooldowns().reset_action_cooldown(wentry.cooldown);
	}

	cooldowns().action_cooldown *= estats.cooldown_mult;
	reset_rest_cooldown();
}

void PlayerInst::use_spell(GameState* gs, const GameAction& action) {
	MTwist& mt = gs->rng();
	EffectiveStats& estats = effective_stats();

	spell_id spell = spells_known().get(action.use_id);
	SpellEntry& spl_entry = game_spell_data.at(spell);

	if (spl_entry.mp_cost > core_stats().mp
			|| (!spl_entry.can_cast_with_cooldown && !cooldowns().can_doaction())) {
		return;
	}

	Pos target = Pos(action.action_x, action.action_y);
	player_use_spell(gs, this, spl_entry, target);

	cooldowns().action_cooldown *= estats.cooldown_mult;
	cooldowns().reset_rest_cooldown(REST_COOLDOWN);
}
