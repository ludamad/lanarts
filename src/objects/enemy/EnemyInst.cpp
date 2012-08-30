/*
 * EnemyInst.cpp:
 *  Represents an AI controlled combat entity
 */

#include <cmath>
#include <typeinfo>

#include "../../display/sprite_data.h"
#include "enemy_data.h"
#include "../../stats/weapon_data.h"

#include "../../display/display.h"

#include "../../stats/stat_formulas.h"

#include "../../serialize/SerializeBuffer.h"

#include "../../display/colour_constants.h"
#include "../../interface/console_description_draw.h"
#include "../../util/math_util.h"
#include "../../lua/LuaValue.h"
#include "../collision_filters.h"

#include "../AnimatedInst.h"

#include "../../gamestate/GameState.h"

#include "EnemyInst.h"
#include "../ItemInst.h"
#include "../ProjectileInst.h"
#include "../player/PlayerInst.h"

//draw depth, also determines what order objects evaluate in
static const int DEPTH = 50;

static EnemyEntry& __E(enemy_id enemytype) {
	return game_enemy_data.at(enemytype);
}

EnemyInst::EnemyInst(int enemytype, int x, int y, int teamid, int mobid) :
		CombatGameInst(__E(enemytype).basestats, __E(enemytype).enemy_sprite,
				teamid, mobid, x, y, __E(enemytype).radius, true, DEPTH), seen(
				false), enemytype(enemytype), xpgain(__E(enemytype).xpaward) {
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

void EnemyInst::signal_was_damaged() {
	eb.randomization.damage_taken_timer = 0;
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
}

void EnemyInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	CombatGameInst::deserialize(gs, serializer);
	serializer.read(seen);
	serializer.read_int(enemytype);
	eb.deserialize(gs, serializer);
	serializer.read_int(xpgain);
	CollisionAvoidance& coll_avoid = gs->collision_avoidance();
	collision_simulation_id() = coll_avoid.add_active_object(pos(),
			target_radius, effective_stats().movespeed);
}
EnemyEntry& EnemyInst::etype() {
	return game_enemy_data.at(enemytype);
}

void EnemyInst::init(GameState* gs) {
	CombatGameInst::init(gs);
	MonsterController& mc = gs->monster_controller();
//	collision_simulation_id() = coll_avoid.add_object(e);
	mc.register_enemy(this);

	CollisionAvoidance& coll_avoid = gs->collision_avoidance();
	collision_simulation_id() = coll_avoid.add_active_object(pos(),
			target_radius, effective_stats().movespeed);

	lua_gameinst_callback(gs->get_luastate(), etype().init_event, this);
}

void EnemyInst::step(GameState* gs) {
	//Much of the monster implementation resides in MonsterController
	CombatGameInst::step(gs);
	update_position();
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
void EnemyInst::draw(GameState* gs) {

	GameView& view = gs->view();
	GLimage& img = game_sprite_data[etype().enemy_sprite].img();

	if (gs->game_settings().draw_diagnostics) {
		char statbuff[255];
		snprintf(statbuff, 255,
				"simid=%d nvx=%f vy=%f\n chasetime=%d \n mdef=%d pdef=%d", // \n act=%d, path_steps = %d\npath_cooldown = %d\n",
				simulation_id, vx, vy, eb.chase_timeout,
				(int) effective_stats().magic.resistance,
				(int) effective_stats().physical.resistance);
		//eb.current_action,
		//eb.path_steps, eb.path_cooldown);
		gl_printf(gs->primary_font(), Colour(255, 255, 255),
				x - radius - view.x, y - 70 - view.y, statbuff);
	}

	int w = img.width, h = img.height;
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;

	if (!seen) {
		seen = true;
		gs->enemies_seen().mark_as_seen(enemytype);
		show_appear_message(gs->game_chat(), etype());
	}

	BBox ebox(xx, yy, xx + w, yy + h);
	if (ebox.contains(gs->mouse_x() + view.x, gs->mouse_y() + view.y)) {
		draw_console_enemy_description(gs, etype());
	}

	CombatGameInst::draw(gs);
}

EnemyInst* EnemyInst::clone() const {
	return new EnemyInst(*this);
}

bool EnemyInst::within_field_of_view(const Pos & pos) {
	return distance_between(Pos(x, y), pos) <= 100;
}

void EnemyInst::die(GameState *gs) {
	if (!destroyed) {
		gs->add_instance(new AnimatedInst(pos(), etype().enemy_sprite, 20));
		gs->remove_instance(this);

		CollisionAvoidance& coll_avoid = gs->collision_avoidance();
		coll_avoid.remove_object(collision_simulation_id());

		show_defeat_message(gs->game_chat(), etype());
		if (etype().death_sprite > -1) {
			const int DEATH_SPRITE_TIMEOUT = 1600;
			gs->add_instance(
					new AnimatedInst(pos(), etype().death_sprite,
							DEATH_SPRITE_TIMEOUT, Posf(), Posf(),
							ItemInst::DEPTH));
		}
	}
}

void EnemyInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(EnemyInst*) inst = *this;
}
