/*
 * EnemyInst.cpp:
 *  Represents an AI controlled combat entity
 */

#include <cmath>
#include <typeinfo>

#include <luawrap/LuaValue.h>
#include <luawrap/calls.h>
#include <lcommon/SerializeBuffer.h>

#include <ldraw/draw.h>
#include <ldraw/Font.h>
#include <ldraw/DrawOptions.h>

#include "lua_api/lua_api.h"

#include "draw/colour_constants.h"
#include "draw/draw_sprite.h"
#include "draw/SpriteEntry.h"

#include "gamestate/GameState.h"
#include "interface/console_description_draw.h"

#include "stats/items/WeaponEntry.h"
#include "stats/stat_formulas.h"

#include <lcommon/math_util.h>
#include "../player/PlayerInst.h"

#include "../AnimatedInst.h"

#include "../ItemInst.h"

#include "../ProjectileInst.h"
#include "../collision_filters.h"

#include "EnemyInst.h"
#include "EnemyEntry.h"

//draw depth, also determines what order objects evaluate in
static const int DEPTH = 50;

static EnemyEntry& __E(enemy_id enemytype) {
	return game_enemy_data.at(enemytype);
}

float monster_difficulty_multiplier(GameState* gs, EnemyEntry& etype) {
	size_t size = gs->player_data().all_players().size();
	if (size > 6) {
		size = 6; // A group larger than 6 will probably be split up considerably
	}
	float mult = log(size); //NB: natural log, base e ~ 2.718...
	if (etype.unique) {
		return 1 + mult / 2; // Can reasonably expect all players to be part of a boss fight
	}
        // For now try with static enemies:
	return 1;  // + mult / 3;
}

EnemyInst::EnemyInst(int enemytype, int x, int y) :
				CombatGameInst(__E(enemytype).basestats,
						__E(enemytype).enemy_sprite, x, y,
						__E(enemytype).radius, true, DEPTH) {
	this->seen = false;
	this->xpgain = __E(enemytype).xpaward;
	this->enemytype = enemytype;
	this->enemy_regen_cooloff = 0;
}

EnemyInst::~EnemyInst() {

}

static void combine_hash(unsigned int& hash, unsigned int val1, unsigned val2) {
	hash ^= (hash >> 11) * val1;
	hash ^= val1;
	hash ^= (hash >> 11) * val2;
	hash ^= val2;
	hash ^= hash << 11;
}

void EnemyInst::signal_attacked_successfully() {
	eb.randomization.successful_hit_timer = 0;
}

unsigned int EnemyInst::integrity_hash() {
	unsigned int hash = CombatGameInst::integrity_hash();
	combine_hash(hash, eb.current_node, eb.path_steps);
	combine_hash(hash, eb.path_start.x, eb.path_start.y);
// TODO: hash the content of the simulation object, the id is a bad thing to hash
//	combine_hash(hash, collision_simulation_id(), eb.current_action);
	return hash;
}

void EnemyInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::serialize(gs, serializer);
	serializer.write(seen);
	serializer.write_int(enemytype);
	eb.serialize(gs, serializer);
	serializer.write_int(xpgain);
	serializer.write_int(enemy_regen_cooloff);
//	ai_state.serialize(gs, serializer);
}

void EnemyInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::deserialize(gs, serializer);
	serializer.read(seen);
	serializer.read_int(enemytype);
	eb.deserialize(gs, serializer);
	serializer.read_int(xpgain);
	serializer.read_int(enemy_regen_cooloff);
	CollisionAvoidance& coll_avoid = gs->collision_avoidance();
	collision_simulation_id() = coll_avoid.add_active_object(ipos(),
			target_radius, effective_stats().movespeed);
//	ai_state.deserialize(gs, serializer);
}

bool EnemyInst::damage(GameState* gs, int dmg) {
	eb.damage_was_taken();
	enemy_regen_cooloff += dmg;

        return CombatGameInst::damage(gs, dmg);
}

EnemyEntry& EnemyInst::etype() {
	return game_enemy_data.at(enemytype);
}

void EnemyInst::init(GameState* gs) {
	CombatGameInst::init(gs);
	MonsterController& mc = gs->monster_controller();
	mc.register_enemy(this);

	CollisionAvoidance& coll_avoid = gs->collision_avoidance();
	collision_simulation_id() = coll_avoid.add_active_object(ipos(),
			target_radius, effective_stats().movespeed);

	lua_State* L = gs->luastate();

	float diffmult = monster_difficulty_multiplier(gs, etype());
	core_stats().hp = ceil(core_stats().hp  * diffmult);
	core_stats().max_hp = ceil(core_stats().max_hp  * diffmult);

	lua_gameinst_callback(L, etype().init_event.get(L), this);
	lua_api::event_monster_init(L, this);
}
static bool starts_with_vowel(const std::string& name) {
	char c = tolower(name[0]);
	return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}
static void show_appear_message(GameChat& chat, EnemyEntry& e) {
	if (e.appear_msg.empty()) {
		char buff[100];
		const char* a_or_an = starts_with_vowel(e.name) ? "An " : "A ";
		snprintf(buff, 100, "%s%s appears!", a_or_an, e.name.c_str());
		chat.add_message(buff, COL_PALE_RED);
	} else {
		chat.add_message(e.appear_msg.c_str(), COL_PALE_RED);
	}
}
static void show_defeat_message(GameChat& chat, EnemyEntry& e) {
	if (!e.defeat_msg.empty()) {
		chat.add_message(e.defeat_msg, COL_MUTED_GREEN);
	}
}

void EnemyInst::step(GameState* gs) {
	//Much of the monster implementation resides in MonsterController

// XXX: Make the monster health absorbing way less hackish and more general
	int hp_before = stats().core.hp;

	CombatGameInst::step(gs);

    // Regenerate much quicker if the player left the level:
	if (!gs->player_data().level_has_player(gs->get_level_id())) {
	    core_stats().heal_hp(effective_stats().core.hpregen * 7, effective_stats().core.max_hp);
	}
    // Absorb health regen if recently damaged
	int hp_gain = std::max(0, stats().core.hp - hp_before);
	int hp_cooloff = std::min(enemy_regen_cooloff, hp_gain);
	enemy_regen_cooloff -= hp_cooloff;
	stats().core.hp -= hp_cooloff;
	effective_stats().core.hp = stats().core.hp;

	update_position();

	if (!seen && gs->object_visible_test(this, gs->local_player())) {
		seen = true;
		gs->enemies_seen().mark_as_seen(enemytype);
                if (gs->local_player()->current_floor == current_floor) {
                    play("sound/see_monster.ogg");
                }
		show_appear_message(gs->game_chat(), etype());
	}
}
void EnemyInst::draw(GameState* gs) {
	GameView& view = gs->view();
	ldraw::Drawable& spr = res::sprite(etype().enemy_sprite);

	if (gs->game_settings().draw_diagnostics) {
		char statbuff[255];
		snprintf(statbuff, 255,
				"simid=%d nvx=%f vy=%f\n chasetime=%d \n mdef=%d pdef=%d", // \n act=%d, path_steps = %d\npath_cooldown = %d\n",
				simulation_id, vx, vy, eb.chase_timeout,
				(int) effective_stats().magic.resistance,
				(int) effective_stats().physical.resistance);
		//eb.current_action,
		//eb.path_steps, eb.path_cooldown);
		gs->font().draw(COL_WHITE, Pos(x - radius - view.x, y - 70 - view.y),
				statbuff);
	}

	int w = spr.size().w, h = spr.size().h;
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;

	if (etype().draw_event.empty()) {
		float frame = gs->frame();
		if (etype().name == "Hydra") {
			frame = floor(core_stats().hp / float(core_stats().max_hp) * 4);
			if (frame >= 4)
				frame = 4;
		}
		CombatGameInst::draw(gs, frame);
	} else {
		lua_State* L = gs->luastate();
		etype().draw_event.get(L).push();
		luawrap::call<void>(L, (GameInst*) this);
	}
}

EnemyInst* EnemyInst::clone() const {
	return new EnemyInst(*this);
}

bool EnemyInst::within_field_of_view(const Pos & pos) {
	return distance_between(Pos(x, y), pos) <= 100;
}

const double deg2rad = 3.14159265 / 180.0;
void EnemyInst::die(GameState *gs) {
	if (!destroyed) {
                // Figure out if the respawns_on_death flag was set
                lua_lookup(gs->luastate(), "respawns_on_death");
                bool should_respawn = lua_toboolean(gs->luastate(), -1);
                lua_pop(gs->luastate(), 1);

		lua_api::event_monster_death(gs->luastate(), this);
                gs->player_data().n_enemy_killed(enemytype)++;
		AnimatedInst* anim = new AnimatedInst(ipos(), etype().enemy_sprite, 20);
		anim->frame(0);
		gs->add_instance(anim);
		gs->remove_instance(this);
                play("sound/paind.ogg");

	        MTwist& mt = gs->rng();
                if (should_respawn) {
                        printf("SHOULD\n");
                    auto* level = gs->get_level();
                    for (int i = 0; i < 100; i++) {
                        double direction = mt.rand(360) * deg2rad;
                        double magnitude = mt.rand(3100) + 100;
                        double vx = cos(direction) * magnitude, vy = sin(direction) * magnitude;
                        int nx = round(rx + vx), ny = round(ry + vy);
                        bool solid = gs->tile_radius_test(nx, ny, TILE_SIZE) || gs->object_radius_test(nx, ny, TILE_SIZE);
                        if (solid || gs->radius_visible_test(nx, ny, TILE_SIZE * 2)) {
                            continue;
                        }
                        if (nx < 0 || ny < 0 || nx > level->width() || ny > level->height()) { 
                            continue;
                        }
                        printf("SPAWNING %i \n", i);
                        gs->add_instance(new EnemyInst(enemytype, nx, ny));
                        break;
                    }
                }
		CollisionAvoidance& coll_avoid = gs->collision_avoidance();
		coll_avoid.remove_object(collision_simulation_id());

		show_defeat_message(gs->game_chat(), etype());
		if (etype().death_sprite > -1) {
			const int DEATH_SPRITE_TIMEOUT = 1600;
			gs->add_instance(
					new AnimatedInst(ipos(), etype().death_sprite,
							DEATH_SPRITE_TIMEOUT, PosF(), PosF(),
							ItemInst::DEPTH));
		}
	}
}

void EnemyInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(EnemyInst*) inst = *this;
}
