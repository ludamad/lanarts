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

#include "lua_api/lua_newapi.h"

#include "../enemy/EnemyInst.h"

#include "../AnimatedInst.h"
#include "../ItemInst.h"
#include "../ProjectileInst.h"
#include "PlayerInst.h"

PlayerInst::PlayerInst(const CombatStats& stats, sprite_id sprite, int x, int y,
		bool local) :
		CombatGameInst(stats, sprite, x, y, RADIUS, true, DEPTH), actions_set_for_turn(
				false), fieldofview(LINEOFSIGHT), local(local), moving(0), autouse_mana_potion_try_count(
				0), previous_spellselect(0), spellselect(-1) {
	last_chosen_weaponclass = "unarmed";
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
	CombatGameInst::init(gs);

	_score_stats.deepest_floor = std::max(_score_stats.deepest_floor, current_floor);

	_path_to_player.initialize(gs->tiles().solidity_map());
	_path_to_player.fill_paths_in_radius(pos(), PLAYER_PATHING_RADIUS);
	collision_simulation_id() = gs->collision_avoidance().add_player_object(
			this);

	lua_api::event_player_init(gs->luastate(), this);
}

PlayerInst::~PlayerInst() {
}

void PlayerInst::update_field_of_view(GameState* gs) {
	int sx = last_x / TILE_SIZE;
	int sy = last_y / TILE_SIZE;
	field_of_view().calculate(gs, sx, sy);
}

bool PlayerInst::within_field_of_view(const Pos& pos) {
	return fieldofview.within_fov(pos.x / TILE_SIZE, pos.y / TILE_SIZE);
}

void PlayerInst::die(GameState* gs) {
	//Let step event handle death
}

void PlayerInst::deinit(GameState* gs) {
	CombatGameInst::deinit(gs);
	current_target = NONE;
	gs->collision_avoidance().remove_object(collision_simulation_id());
}

static Pos seen_square_in_area(MTwist& mt, GameTiles& tiles) {
	Pos p, ret;
	do {
		p.x = mt.rand(tiles.tile_width());
		p.y = mt.rand(tiles.tile_height());
	} while (!tiles.is_seen(p) || tiles.is_solid(p));
	return centered_multiple(p, TILE_SIZE);
}

// Assumes overworld == map ID 0
static void spawn_in_overworld(GameState* gs, PlayerInst* player) {
	int current_map = gs->game_world().get_current_level_id();
	int overworld_map = 0;
	GameMapState* overworld = gs->game_world().get_level(overworld_map);
	Pos sqr = seen_square_in_area(gs->rng(), overworld->tiles());

	for (int i = 0; i < gs->player_data().all_players().size(); i++) {
		PlayerDataEntry& pde = gs->player_data().all_players()[i];
		if (pde.player_inst.get() == player) {
			pde.action_queue.clear_actions();
		}
	}

	gs->game_chat().add_message(
			player->is_local_player() ?
					"You have respawned!" : "Your ally has respawned!",
			Colour(100, 150, 150));
	if (current_map != overworld_map) {
		gs->game_world().level_move(player->id, sqr.x, sqr.y, current_map,
				overworld_map);
	} else {
		player->update_position(sqr.x, sqr.y);
		gs->view().sharp_center_on(player->pos());
	}
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
		if (isvisible) {
			current_target = e->id;
			return;
		}
		j++;

	}

}

void PlayerInst::step(GameState* gs) {
	perf_timer_begin(FUNCNAME);
	_path_to_player.fill_paths_in_radius(pos(), PLAYER_PATHING_RADIUS);

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
		printf("No actions for turn player id %d\n", id);
		perf_timer_end(FUNCNAME);
		return;
	}

	vx = 0, vy = 0;

	CombatGameInst::step(gs);
	GameView& view = gs->view();

	//Stats/effect step
	if (cooldowns().is_hurting())
		reset_rest_cooldown();

	if (stats().has_died()) {
		lua_api::event_player_death(gs->luastate(), this);

		_score_stats.deaths++;
		queued_actions.clear();
		actions_set_for_turn = false;
		if (gs->game_settings().regen_on_death) {
			stats().effects.clear();
			stats().core.heal_fully();
			spawn_in_overworld(gs, this);
		} else {
			gs->game_world().reset(0);
		}
		perf_timer_end(FUNCNAME);
		return;
	}

	is_resting = false;
	perform_queued_actions(gs);
	vx = round(vx * 256.0f) / 256.0f;
	vy = round(vy * 256.0f) / 256.0f;

	if (!gs->key_down_state(SDLK_x) && is_local_player())
		view.center_on(last_x, last_y);

	update_position(rx + vx, ry + vy);
	perf_timer_end(FUNCNAME);
}

void PlayerInst::draw(GameState* gs) {
	CombatGameInst::draw(gs);
}

void PlayerInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(PlayerInst*) inst = *this;
}

void PlayerInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::serialize(gs, serializer);
	serializer.write(_score_stats);
	serializer.write(actions_set_for_turn);
	serializer.write(last_chosen_weaponclass);
//	serializer.write_container(queued_actions);

	SERIALIZE_POD_REGION(serializer, this, local, spellselect);
}

void PlayerInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::deserialize(gs, serializer);
	serializer.read(_score_stats);
	serializer.read(actions_set_for_turn);
	serializer.read(last_chosen_weaponclass);
//	serializer.read_container(queued_actions);
	queued_actions.clear();
	_path_to_player.initialize(gs->tiles().solidity_map());
	_path_to_player.fill_paths_in_radius(pos(), PLAYER_PATHING_RADIUS);
	update_field_of_view(gs);
	DESERIALIZE_POD_REGION(serializer, this, local, spellselect);

	CollisionAvoidance& coll_avoid = gs->collision_avoidance();
	collision_simulation_id() = coll_avoid.add_player_object(this);
}

PlayerInst *PlayerInst::clone() const {
	return new PlayerInst(*this);
}

