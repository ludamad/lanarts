/*
 * EnemyInst.cpp:
 *  Represents an AI controlled combat entity
 */

#include <cmath>
#include <typeinfo>

#include "../../data/sprite_data.h"
#include "../../data/enemy_data.h"
#include "../../data/weapon_data.h"

#include "../../display/display.h"

#include "../../gamestats/stat_formulas.h"

#include "../../util/world/collision_util.h"
#include "../../util/math_util.h"
#include "../../util/LuaValue.h"

#include "../utility_objects/AnimatedInst.h"

#include "../GameState.h"

#include "EnemyInst.h"
#include "ItemInst.h"
#include "ProjectileInst.h"
#include "PlayerInst.h"

//draw depth, also determines what order objects evaluate in
static const int DEPTH = 50;

static EnemyEntry& __E(enemy_id enemytype) {
	return game_enemy_data.at(enemytype);
}

EnemyInst::EnemyInst(int enemytype, int x, int y) :
		CombatGameInst(__E(enemytype).basestats, __E(enemytype).enemy_sprite, 0,
				x, y, __E(enemytype).radius, true, DEPTH), seen(false), enemytype(
				enemytype), xpgain(__E(enemytype).xpaward) {
}

EnemyInst::~EnemyInst() {
}

EnemyEntry& EnemyInst::etype() {
	return game_enemy_data.at(enemytype);
}

void EnemyInst::init(GameState* gs) {
	CombatGameInst::init(gs);
	MonsterController& mc = gs->monster_controller();
	mc.register_enemy(this);

//	int ln = gs->level()->level_number + 1;
//	core_stats().hp += core_stats().hp * ln / 10.0;
//	core_stats().max_hp += core_stats().max_hp * ln / 10.0;
//	core_stats().mp += core_stats().mp * ln / 10.0;
//	core_stats().max_mp += core_stats().max_mp * ln / 10.0;

	lua_gameinstcallback(gs->get_luastate(), etype().init_event, id);
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
		chat.add_message(buff, Colour(255, 148, 120));
	} else {
		chat.add_message(e.appear_msg.c_str(), Colour(255, 148, 120));
	}
}
static void show_defeat_message(GameChat& chat, EnemyEntry& e) {
	if (!e.defeat_msg.empty()) {
		chat.add_message(e.defeat_msg, Colour(50, 205, 50));
	}

}
void EnemyInst::draw(GameState* gs) {

	GameView& view = gs->window_view();
	GLimage& img = game_sprite_data[etype().enemy_sprite].img();

	if (gs->game_settings().draw_diagnostics) {
		char statbuff[255];
		snprintf(
				statbuff,
				255,
				"vx=%f vy=%f\n act=%d, path_steps = %d\npath_cooldown = %d\nradius=%d\ntarget_radius=%d",
				vx, vy, eb.current_action, eb.path_steps, eb.path_cooldown,
				radius, target_radius);
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
		show_appear_message(gs->game_chat(), etype());
	}

	CombatGameInst::draw(gs);
}

//void EnemyInst::attack(GameState* gs, GameInst* inst, AttackStats& attack) {
//	if (!cooldowns().can_doaction())
//		return;
//	CombatGameInst* combat_inst;
//	if ((combat_inst = dynamic_cast<CombatGameInst*>(inst))) {
//		if (attack.is_ranged()) {
//			ProjectileEntry& pentry = attack.projectile_entry();
//
//			Pos p(combat_inst->x, combat_inst->y);
//			p.x += gs->rng().rand(-12, +13);
//			p.y += gs->rng().rand(-12, +13);
//			if (gs->tile_radius_test(p.x, p.y, 10)) {
//				p.x = combat_inst->x;
//				p.y = combat_inst->y;
//			}
//			//	ProjectileInst(sprite_id sprite, obj_id originator, float speed, int range,
//			//			int damage, int x, int y, int tx, int ty, bool bounce = false,
//			//			int hits = 1, obj_id target = NONE);
//			GameInst* bullet = new ProjectileInst(pentry.attack_sprite, id,
//					pentry.speed, pentry.range, 1, x, y, p.x, p.y);
//			gs->add_instance(bullet);
//			cooldowns().reset_action_cooldown(pentry.cooldown);
//			cooldowns().action_cooldown += gs->rng().rand(-4, 5);
//		} else {
//			WeaponEntry& wentry = attack.weapon_entry();
//
//			int damage = physical_damage_formula(effective_stats(),
//					combat_inst->effective_stats());
//			if (!gs->game_settings().invincible)
//				combat_inst->damage(gs, damage);
//
//			char dmgstr[32];
//			snprintf(dmgstr, 32, "%d", damage);
//			float rx, ry;
//			direction_towards(Pos(x, y), Pos(combat_inst->x, combat_inst->y),
//					rx, ry, 0.5);
//			gs->add_instance(
//					new AnimatedInst(combat_inst->x - 5 + rx * 5,
//							combat_inst->y + ry * 5, -1, 25, rx, ry, dmgstr,
//							Colour(255, 148, 120)));
//
//			cooldowns().reset_action_cooldown(wentry.cooldown);
//			cooldowns().action_cooldown += gs->rng().rand(-4, 5);
//		}
//	}
//
//}

EnemyInst* EnemyInst::clone() const {
	return new EnemyInst(*this);
}

bool EnemyInst::within_field_of_view(const Pos & pos) {
	return distance_between(Pos(x, y), pos) <= 100;
}

void EnemyInst::die(GameState *gs) {
	if (!destroyed) {
		gs->add_instance(new AnimatedInst(x, y, etype().enemy_sprite, 15));
		gs->monster_controller().deregister_enemy(this);
		gs->remove_instance(this);
		show_defeat_message(gs->game_chat(), etype());
		if (etype().death_sprite > -1) {
//			gs->add_instance(new AnimatedInst(x, y, etype().death_sprite, 400, ItemInst::DEPTH));
		}
	}
}

void EnemyInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(EnemyInst*) inst = *this;
}
