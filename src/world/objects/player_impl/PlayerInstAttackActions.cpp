/*
 * PlayerInstAttackActions.cpp
 *  Implements the various attack and misc spell actions the player can perform.
 */

extern "C" {
#include <lua/lua.h>
}

#include "../../../data/item_data.h"
#include "../../../data/spell_data.h"
#include "../../../data/sprite_data.h"
#include "../../../data/tile_data.h"
#include "../../../data/projectile_data.h"
#include "../../../data/weapon_data.h"

#include "../../../display/display.h"

#include "../../../lua/lua_api.h"

#include "../../../util/colour_constants.h"
#include "../../../util/math_util.h"
#include "../../../util/game_basic_structs.h"
#include "../../../util/world/collision_util.h"

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
		float dist = sqrt(dx * dx + dy * dy) - inst->target_radius;
		if (closest == NULL || dist < mindist) {
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
	PlayerController& pc = gs->player_controller();

	std::vector<GameInst*> visible_monsters;
	get_visible_monsters(gs, visible_monsters);

	int maxdist = 0;
	for (int i = 0; i < pc.player_ids().size(); i++) {
		PlayerInst* player = (PlayerInst*) gs->get_instance(pc.player_ids()[i]);
		BBox fbox = player->field_of_view().tiles_covered();
		FOR_EACH_BBOX(fbox, x, y) {
			if (player->field_of_view().within_fov(x, y)) {
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

void PlayerInst::queue_io_spell_and_attack_actions(GameState* gs, float dx,
		float dy) {
	GameView& view = gs->window_view();
	WeaponEntry& wentry = weapon().weapon_entry();

	bool mouse_within = gs->mouse_x() < gs->window_view().width;
	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	int level = gs->get_level()->roomid, frame = gs->frame();
	bool spell_used = false;

	//Keyboard-oriented blink
	if (!spell_used && gs->key_press_state(SDLK_h)) {
		bool canuse = true;
		Pos blinkposition;
		if (core_stats().mp < 50) {
			canuse = false;
			gs->game_chat().add_message(
					"You do not have enough mana to blink, 50 MP required!",
					COL_RED);
		} else if (!find_safest_square(this, gs, blinkposition)) {
			canuse = false;
			gs->game_chat().add_message(
					"Cannot auto-blink, no hostile targets. Right click to blink manually.");
		}
		if (canuse) {
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
		bool use_weapon = spellselect == -1 || core_stats().mp < mpcost;
		if (use_weapon) {
			target = get_weapon_autotarget(gs, this, target, dx, dy);
		}
		if (target && use_weapon) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_WEAPON, frame, level,
							spellselect, target->x, target->y));
			spell_used = true;
		} else if (target) {
			if (cooldowns().can_doaction() && core_stats().mp >= mpcost
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
		if (core_stats().mp >= 50) {
			if (!gs->solid_test(this) && gs->object_visible_test(this)) {
				queued_actions.push_back(
						GameAction(id, GameAction::USE_SPELL, frame, level, 2,
								x, y));
			}
		} else {
			gs->game_chat().add_message(
					"You do not have enough mana to blink, 50 MP required!",
					COL_RED);
		}
		/* restore x and y */
		x = px, y = py;
	}

	if (!spell_used && gs->mouse_left_down() && mouse_within) {
		int mpcost = 10;
		if (spellselect)
			mpcost = 20;
		if (spellselect == -1 || core_stats().mp < mpcost) {
			bool is_projectile = wentry.uses_projectile
					|| equipment().has_projectile();
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
		} else if (cooldowns().can_doaction() && core_stats().mp >= mpcost) {
			queued_actions.push_back(
					GameAction(id, GameAction::USE_SPELL, frame, level,
							spellselect, rmx, rmy));
			spell_used = true;
		}
	}
}

void PlayerInst::use_weapon(GameState *gs, const GameAction& action) {
	WeaponEntry& wentry = weapon().weapon_entry();
	MTwist& mt = gs->rng();
	const int MAX_MELEE_HITS = 10;
	EffectiveStats& estats = effective_stats();
	if (!cooldowns().can_doaction())
		return;

	Pos start(x, y);
	Pos actpos(action.action_x, action.action_y);

	if (wentry.uses_projectile && !equipment().has_projectile())
		return;

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

		if (numhit == 0)
			return;

		for (int i = 0; i < numhit; i++) {
			EnemyInst* e = (EnemyInst*) enemies[i];
			if (attack(gs, e, AttackStats(equipment().weapon))) {
				char buffstr[32];
				gain_xp(gs, e->xpworth());
				if (is_local_player()) {
					snprintf(buffstr, 32, "%d XP", e->xpworth());
					gs->add_instance(
							new AnimatedInst(e->x - 5, e->y - 5, -1, 25, 0, 0,
									AnimatedInst::DEPTH, buffstr,
									Colour(255, 215, 11)));
				}
			}
		}
		cooldowns().reset_action_cooldown(wentry.cooldown);
	}

	cooldowns().action_cooldown *= estats.cooldown_mult;
	reset_rest_cooldown();
}
//
//static void __use_projectile_spell(GameState* gs, PlayerInst* p,
//		SpellEntry& spl_entry, const Projectile& projectile,
//		const Pos& target) {
//	MTwist& mt = gs->rng();
//	AttackStats projectile_attack(Weapon(), projectile);
//	ProjectileEntry& pentry = projectile.projectile_entry();
//	bool wallbounce = pentry.can_wall_bounce;
//	int nbounces = pentry.number_of_target_bounces;
//
//	GameInst* pinst = new ProjectileInst(projectile,
//			p->effective_atk_stats(mt, projectile_attack), p->id,
//			Pos(p->x, p->y), target, pentry.speed, pentry.range, NONE,
//			wallbounce, nbounces);
//	gs->add_instance(pinst);
//}
//
//static void __use_luacallback_spell(GameState* gs, PlayerInst* p,
//		SpellEntry& spl_entry, LuaValue& action, const Pos& target) {
//
//}
//
//static void __use_spell(GameState* gs, PlayerInst* p, SpellEntry& spl_entry,
//		const Pos& target) {
//	p->core_stats().mp -= spl_entry.mp_cost;
//	p->cooldowns().reset_action_cooldown(spl_entry.cooldown);
//	if (spl_entry.uses_projectile()) {
//		__use_projectile_spell(gs, p, spl_entry, spl_entry.projectile, target);
//	} else {
//		__use_luacallback_spell(gs, p, spl_entry, spl_entry.action, target);
//	}
//}
//
//void PlayerInst::use_spell(GameState* gs, const GameAction& action) {
//	MTwist& mt = gs->rng();
//	EffectiveStats& estats = effective_stats();
//	if (action.use_id < 2 && !cooldowns().can_doaction())
//		return;
//
//	spell_id spell;
//
//	if (action.use_id == 0) {
//		spell = get_spell_by_name("Fire Bolt");
//	} else if (action.use_id == 1) {
//		spell = get_spell_by_name("Magic Blast");
//	} else if (action.use_id == 2) {
//		spell = get_spell_by_name("Blink");
//	}
//
//	Pos target = Pos(action.action_x, action.action_y);
//	__use_spell(gs, this, game_spell_data.at(spell), target);
//
//	if (action.use_id == 0) {
//		double mult = 1 + (class_stats().xplevel - 1) / 10.0;
//		mult = std::min(2.0, mult);
//		cooldowns().action_cooldown /= mult;
//	}
//	cooldowns().action_cooldown *= estats.cooldown_mult;
//	cooldowns().reset_rest_cooldown(REST_COOLDOWN);
//}
void PlayerInst::use_spell(GameState* gs, const GameAction& action) {
	MTwist& mt = gs->rng();
	EffectiveStats& estats = effective_stats();
	if (action.use_id < 2 && !cooldowns().can_doaction())
		return;

	Projectile projectile;

	if (action.use_id == 0) {
		projectile = Projectile(get_projectile_by_name("Fire Bolt"));
		core_stats().mp -= 10;
	} else if (action.use_id == 1) {
		projectile = Projectile(get_projectile_by_name("Magic Blast"));
		core_stats().mp -= 20;
	} else if (action.use_id == 2) {
		core_stats().mp -= 50;
	}

	bool bounce = true;
	int hits = 0;

	if (action.use_id == 1) {
	//		atk.attack_sprite = get_sprite_by_name("magic blast");
	//		atk.projectile_speed /= 1.75;
		//	atk.damage *= 2;
		bounce = false;
		hits = 3;
	}
	//	atk.damage = estats.calculate_spell_damage(gs->rng(), action.use_id);

	if (action.use_id < 2) {
		Pos self(x, y), target(action.action_x, action.action_y);
		ProjectileEntry& pentry = projectile.projectile_entry();
		AttackStats spellattack(Weapon(), projectile);
		GameInst* bullet = new ProjectileInst(projectile,
				effective_atk_stats(mt, spellattack), id, self, target,
				pentry.speed, pentry.range, NONE, bounce, hits);
		gs->add_instance(bullet);

		cooldowns().reset_action_cooldown(pentry.cooldown * 1.4);
	} else {
		update_position(action.action_x, action.action_y);
	}

	if (action.use_id == 0) {
		double mult = 1 + (class_stats().xplevel - 1) / 10.0;
		mult = std::min(2.0, mult);
		cooldowns().action_cooldown /= mult;
	} else if (action.use_id == 2) {
		cooldowns().reset_action_cooldown(130);
	}
	cooldowns().action_cooldown *= estats.cooldown_mult;
	cooldowns().reset_rest_cooldown(REST_COOLDOWN);
}
