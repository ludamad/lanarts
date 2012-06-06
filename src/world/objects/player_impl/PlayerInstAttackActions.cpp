/*
 * PlayerInstAttackActions.cpp
 *  Implements the various attack and misc spell actions the player can perform.
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

static GameInst* get_weapon_autotarget(GameState* gs, PlayerInst* p,
		GameInst* targ, float dx, float dy) {
	WeaponEntry& wentry = game_weapon_data[p->weapon_type()];
	Pos ppos(p->x, p->y);
	GameInst* inst = NULL;
	bool ismelee = !wentry.projectile;
	int target_range = wentry.range + p->radius;

	if (targ && distance_between(Pos(targ->x, targ->y), ppos) <= target_range) {
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
	} else
		return false;
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

static void projectile_item_drop(GameState* gs, GameInst* obj, void* data) {
	item_id item_type = (item_id)(long)data;
	ItemEntry& ientry = game_item_data[item_type];
	if (ientry.equipment_type == ItemEntry::PROJECTILE) {
		ProjectileEntry& pentry = game_projectile_data[ientry.equipment_id];
		int break_roll = gs->rng().rand(100);
		if (break_roll < pentry.break_chance) {
			return; // Item 'breaks', ie don't spawn new item
		}
	}

	int dropx = round_to_multiple(obj->x, TILE_SIZE, true), dropy =
			round_to_multiple(obj->y, TILE_SIZE, true);

	ItemInst* item = new ItemInst(item_type, dropx, dropy);
	gs->add_instance(item);
}

void PlayerInst::queue_io_spell_and_attack_actions(GameState* gs, float dx,
		float dy) {
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
		bool use_weapon = spellselect == -1 || stats().mp < mpcost;
		if (use_weapon) {
			target = get_weapon_autotarget(gs, this, target, dx, dy);
		}
		if (target && use_weapon) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_WEAPON, frame, level,
							spellselect, target->x, target->y));
			spell_used = true;
		} else if (target) {
			if (!stats().has_cooldown() && stats().mp >= mpcost
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
		/* set-up x and y for gs->object_visible_test() */
		x = rmx, y = rmy;
		if (stats().mp >= 50 && !gs->solid_test(this)
				&& gs->object_visible_test(this)) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_SPELL, frame, level, 2, x,
							y));
		}
		/* restore x and y */
		x = px, y = py;
	}

	if (!spell_used && gs->mouse_left_down() && mouse_within) {
		int mpcost = 10;
		if (spellselect)
			mpcost = 20;
		if (spellselect == -1 || stats().mp < mpcost) {
			bool is_projectile = !game_weapon_data[weapon_type()].projectile;
			GameInst* target = NULL;
			if (!is_projectile) {
				target = get_weapon_autotarget(gs, this, NULL, rmx - x,
						rmy - y);
				if (target) {
					rmx = target->x, rmy = target->y;
				}
			}

			if (is_projectile || target) {
				queued_actions.push_back(
						GameAction(id, GameAction::USE_WEAPON, frame, level,
								spellselect, rmx, rmy));
			}
			spell_used = true;
		} else if (!stats().has_cooldown() && stats().mp >= mpcost) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_SPELL, frame, level,
							spellselect, rmx, rmy));
			spell_used = true;
		}
	}
}

void PlayerInst::use_weapon(GameState *gs, const GameAction& action) {
	WeaponEntry& wtype = game_weapon_data[weapon_type()];
	MTwist& mt = gs->rng();
	const int MAX_MELEE_HITS = 10;
	Stats estats = effective_stats(gs->get_luastate());
	if (estats.has_cooldown())
		return;

	Pos actpos(action.action_x, action.action_y);
	int weaprange = wtype.range + this->radius;
	float mag = distance_between(actpos, Pos(x, y));
	if (mag > weaprange) {
		float dx = actpos.x - x, dy = actpos.y - dy;
		actpos = Pos(x + dx / mag * weaprange, y + dy / mag * weaprange);
	}

	if (wtype.projectile && equipment.projectile == -1)
		return;

	if (wtype.projectile) {
		int damage = estats.calculate_melee_damage(mt, weapon_type());
		ProjectileEntry& pentry = game_projectile_data[equipment.projectile];
		item_id item = get_item_by_name(pentry.name.c_str());

		equipment.use_ammo();
		int dmg_bonus = mt.rand(pentry.damage_bonus);

		ObjCallback drop_callback(projectile_item_drop, (void*)item);

		GameInst* bullet = new ProjectileInst(pentry.attack_sprite, id, 6,
				wtype.range, damage + dmg_bonus, x, y, actpos.x, actpos.y,
				false, 0, NONE, drop_callback);
		gs->add_instance(bullet);
	} else {
		GameInst* enemies[MAX_MELEE_HITS];

		int max_targets = std::min(MAX_MELEE_HITS, wtype.max_targets);

		int numhit = get_targets(gs, this, actpos.x, actpos.y, wtype.dmgradius,
				enemies, max_targets);

		if (numhit == 0)
			return;

		for (int i = 0; i < numhit; i++) {
			EnemyInst* e = (EnemyInst*)enemies[i];

			int damage = estats.calculate_melee_damage(mt, weapon_type());
			char buffstr[32];
			snprintf(buffstr, 32, "%d", damage);
			float rx, ry;
			direction_towards(Pos(x, y), Pos(e->x, e->y), rx, ry, 0.5);
			gs->add_instance(
					new AnimatedInst(e->x - 5 + rx * 5, e->y - 3 + rx * 5, -1,
							25, rx, ry, buffstr, Colour(255, 148, 120)));

			if (e->hurt(gs, damage)) {
				gain_xp(gs, e->xpworth());
				if (is_local_player()) {
					snprintf(buffstr, 32, "%d XP", e->xpworth());
					gs->add_instance(
							new AnimatedInst(e->x - 5, e->y - 5, -1, 25, 0, 0,
									buffstr, Colour(255, 215, 11)));
				}
			}
			gs->add_instance(
					new AnimatedInst(e->x, e->y, wtype.attack_sprite, 25));

		}
	}

	stats().cooldown = wtype.cooldown;
	reset_rest_cooldown();
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

	reset_rest_cooldown();
}
