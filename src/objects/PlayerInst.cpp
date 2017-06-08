/*
 * PlayerInst.cpp
 *  Implements the standard GameInst actions such as step, draw, etc for PlayerInst
 */

#include <typeinfo>

#include <lcommon/SerializeBuffer.h>

#include "draw/draw_sprite.h"

#include "draw/SpriteEntry.h"
#include "draw/TileEntry.h"
#include "gamestate/GameState.h"

#include "stats/items/WeaponEntry.h"

#include "stats/ClassEntry.h"
#include "stats/items/ItemEntry.h"

#include <lcommon/math_util.h>

#include "lua_api/lua_api.h"
#include "stats/effect_data.h"

#include "objects/EnemyInst.h"

#include "objects/AnimatedInst.h"
#include "objects/ItemInst.h"
#include "objects/ProjectileInst.h"
#include "objects/PlayerInst.h"

PlayerInst::PlayerInst(const CombatStats& stats, sprite_id sprite, Pos xy, team_id team, bool local) :
		CombatGameInst(stats, sprite, xy, team, RADIUS, true, DEPTH), actions_set_for_turn(
				false), local(local), moving(0), autouse_mana_potion_try_count(
				0), previous_spellselect(0), spellselect(-1) {
	last_chosen_weaponclass = "unarmed";
	field_of_view = new fov();
}


PlayerDataEntry& PlayerInst::player_entry(GameState* gs) const {
	std::vector<PlayerDataEntry>& players = gs->player_data().all_players();
	for (int i = 0; i < players.size(); i++) {
		if (players[i].player_inst.get() == this) {
			return players[i];
		}
	}
	LANARTS_ASSERT(false);
	/* Typematch to quell warning */
	return players.at(-1);
}

void PlayerInst::init(GameState* gs) {

	_score_stats.deepest_floor = std::max(_score_stats.deepest_floor, current_floor);

        if (!_paths_to_object) {
            _paths_to_object = new FloodFillPaths();
        }
	paths_to_object().initialize(gs->tiles().solidity_map());
	paths_to_object().fill_paths_in_radius(ipos(), PLAYER_PATHING_RADIUS);
	collision_simulation_id() = gs->collision_avoidance().add_player_object(
			this);
	// Must set this BEFORE calling CombatGameInst::init:
	vision_radius = gs->get_level()->vision_radius();

    CombatGameInst::init(gs);
    lua_api::event_player_init(gs->luastate(), this);
}

PlayerInst::~PlayerInst() {
}

money_t& PlayerInst::gold(GameState* gs) {
        return gs->player_data().money();
}

void PlayerInst::update_field_of_view(GameState* gs) {
	int sx = last_x / TILE_SIZE;
	int sy = last_y / TILE_SIZE;
	field_of_view->calculate(gs, (is_ghost() ? 3 : gs->game_world().get_current_level()->vision_radius()), sx, sy);
}

bool PlayerInst::within_field_of_view(const Pos& pos) {
	return field_of_view->within_fov(pos.x / TILE_SIZE, pos.y / TILE_SIZE);
}

void PlayerInst::die(GameState* gs) {
	//Let step event handle death
}

void PlayerInst::deinit(GameState* gs) {
	CombatGameInst::deinit(gs);
	current_target = NONE;
	gs->collision_avoidance().remove_object(collision_simulation_id());
}

//Either finds new or shifts target
void PlayerInst::shift_autotarget(GameState* gs) {
	const std::vector<obj_id>& mids = gs->monster_controller().monster_ids();
	int i = 0, j = 0;
	bool has_target = current_target != NONE;
	if (has_target) {
		for (i = 0; i < mids.size(); i++) {
			if (mids[i] == current_target) {
				break;
			}
		}
		j = i + 1;
	}

	while (true) {
		if (j >= mids.size()) {
			// Exit when we end the list if we didn't have a target
			if (!has_target) {
				return;
			}
			j -= mids.size();
		}
		if (j == i && has_target) {
			// Exit when we wrap around if we have a target
			return;
		}
		EnemyInst* e = (EnemyInst*) gs->get_instance(mids[j]);
		bool isvisible = e != NULL && gs->object_visible_test(e, this, false);
		if (isvisible && e->team != team) {
			current_target = e->id;
			return;
		}
		j++;

	}

}

void PlayerInst::step(GameState* gs) {
    PERF_TIMER();
    paths_to_object().fill_paths_in_radius(ipos(), PLAYER_PATHING_RADIUS);
    //if (cooldowns().action_cooldown > 0)
    //printf("MELEE COOLDOWN %d\n", cooldowns().action_cooldown);

	GameInst* target_inst = gs->get_instance(current_target);
	bool visible = target_inst != NULL
			&& gs->object_visible_test(target_inst, this, false);
	if (!visible) {
		current_target = NONE;
	}
	if (current_target == NONE) {
		shift_autotarget(gs);
	}

	if (!actions_set_for_turn) {
		GameInst::step(gs); // For callback
		return;
	}

	vx = 0, vy = 0;

	CombatGameInst::step(gs);
	GameView& view = gs->view();

	//Stats/effect step
	if (cooldowns().is_hurting())
		reset_rest_cooldown();

	if (stats().has_died() && !is_ghost()) {
		bool game_should_end = lua_api::event_player_death(gs->luastate(), this);
		_score_stats.deaths++;
                if (game_should_end) {
                    // End the game:
                    queued_actions.clear();
                    actions_set_for_turn = false;
                    // Queue a restart:
                    gs->game_world().reset();
                    // Exit, game's over, nothing more to see here folks:
                    return;
                }
	}

	is_resting = false;
	perform_queued_actions(gs);
	vx = round(vx * 256.0f) / 256.0f;
	vy = round(vy * 256.0f) / 256.0f;

	if (!gs->key_down_state(SDLK_x) && is_local_player())
		view.center_on(last_x, last_y);

	update_position(rx + vx, ry + vy);
        if (is_ghost()) {
            stats().core.hp = 0;
            stats().core.mp = 0;
            effective_stats().core.hp = 0;
            effective_stats().core.mp = 0;
            reset_rest_cooldown();
        }
}

void PlayerInst::draw(GameState* gs) {
	CombatGameInst::draw(gs, 0.0f, is_ghost() ? 0.4 : 1.0);
}

void PlayerInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(PlayerInst*) inst = *this;
}

bool PlayerInst::can_benefit_from_rest() {
    // If we are not allowed to rest, we therefore cannot benefit.
    if (!effective_stats().allowed_actions.can_use_rest || !effects.can_rest()) {
        return false;
    }
    CoreStats& ecore = effective_stats().core;
    int emax_hp = ecore.max_hp, emax_mp = ecore.max_mp;
    bool atfull = (core_stats().hp >= emax_hp || ecore.hpregen == 0) && (core_stats().mp >= emax_mp || ecore.mpregen == 0);
    if (!atfull) {
        return true;
    }
    // Since spell cooldowns: Keep resting if a spell cooldown is active.
    for (auto& e : cooldowns().spell_cooldowns) {
        if (e.second > 0) {
            return true;
        }
    }
    return false;
}

void PlayerInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::serialize(gs, serializer);
	serializer.write(_score_stats);
	serializer.write(actions_set_for_turn);
    serializer.write(last_chosen_weaponclass);
    serializer.write(_last_moved_direction);
//	serializer.write_container(queued_actions);

	SERIALIZE_POD_REGION(serializer, this, local, spellselect);
}

void PlayerInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::deserialize(gs, serializer);
        delete field_of_view;
	field_of_view = new fov();
	serializer.read(_score_stats);
	serializer.read(actions_set_for_turn);
	serializer.read(last_chosen_weaponclass);
    serializer.read(_last_moved_direction);
//	serializer.read_container(queued_actions);
	queued_actions.clear();
	paths_to_object().initialize(gs->tiles().solidity_map());
	paths_to_object().fill_paths_in_radius(ipos(), PLAYER_PATHING_RADIUS);
	//update_field_of_view(gs);
	DESERIALIZE_POD_REGION(serializer, this, local, spellselect);

	CollisionAvoidance& coll_avoid = gs->collision_avoidance();
	collision_simulation_id() = coll_avoid.add_player_object(this);
}

PlayerInst *PlayerInst::clone() const {
	return new PlayerInst(*this);
}

std::vector<StatusEffect> PlayerInst::base_status_effects(GameState* gs) {
    effect_id id = get_effect_by_name(class_stats().class_entry().name.c_str());
    return {{id, LuaValue(gs->luastate())}};
}
