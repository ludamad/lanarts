#include <cmath>
#include <typeinfo>

#include "EnemyInst.h"
#include "ProjectileInst.h"
#include "PlayerInst.h"
#include "../utility_objects/AnimatedInst.h"

#include "../GameState.h"

#include "../../data/sprite_data.h"
#include "../../data/enemy_data.h"

#include "../../display/display.h"

#include "../../util/collision_util.h"
#include "../../util/math_util.h"
#include "../../util/LuaValue.h"

//draw depth, also determines what order objects evaluate in
static const int DEPTH = 50;

EnemyInst::EnemyInst(int enemytype, int x, int y) :
		GameInst(x, y, game_enemy_data[enemytype].radius, true, DEPTH), seen(
				false), rx(x), ry(y), enemytype(enemytype), eb(
				game_enemy_data[enemytype].basestats.movespeed), xpgain(
				game_enemy_data[enemytype].xpaward), stat(
				game_enemy_data[enemytype].basestats) {
}

EnemyInst::~EnemyInst() {
}

EnemyEntry* EnemyInst::etype() {
	return &game_enemy_data[enemytype];
}

void EnemyInst::init(GameState* gs) {
	MonsterController& mc = gs->monster_controller();
	mc.register_enemy(this);

	//xpgain *=1+gs->branch_level()/10.0;
	//xpgain = round(xpgain/5.0)*5;
	int ln = gs->level()->level_number + 1;
	stats().hp += stats().hp * ln / 10.0;
	stats().max_hp += stats().max_hp * ln / 10.0;
	stats().mp += stats().mp * ln / 10.0;
	stats().max_mp += stats().max_mp * ln / 10.0;
	stats().magicatk.cooldown /= 1.0 + ln / 10.0;
	stats().magicatk.damage *= 1.0 + ln / 10.0;
	stats().magicatk.projectile_speed *= 1.0 + ln / 10.0;
	stats().meleeatk.cooldown /= 1.0 + ln / 10.0;
	stats().meleeatk.damage *= 1.0 + ln / 10.0;
	stats().max_mp += stats().max_mp * ln / 10.0;
	double speedfactor = 1 + stats().max_mp * ln / 10.0;
	if (stats().movespeed < 3)
		eb.speed = std::min(stats().movespeed * speedfactor, 3.0);

	lua_gameinstcallback(gs->get_luastate(), etype()->init_event, id);
}

void EnemyInst::step(GameState* gs) {
	//Much of the monster implementation resides in MonsterController
	stats().step();
}
static bool starts_with_vowel(const char* name){
	char c = tolower(*name);
	return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}
static void show_appear_message(GameChat& chat, EnemyEntry* e){
	char buff[100];
	const char* a_or_an = starts_with_vowel(e->name) ? "An" : "a";

	snprintf(buff, 100, "%s %s appears!", a_or_an, e->name);
	chat.add_message(buff, Colour(255, 248, 220));

}
void EnemyInst::draw(GameState* gs) {

	GameView& view = gs->window_view();
	GLimage& img = game_sprite_data[etype()->sprite_number].img;

	if (gs->game_settings().draw_diagnostics) {
		char statbuff[255];
		snprintf(statbuff, 255,
				"vx=%f vy=%f\n act=%d, steps = %d\ncooldown = %d", eb.vx, eb.vy,
				eb.current_action, eb.path_steps, eb.path_cooldown);
		gl_printf(gs->primary_font(), Colour(255, 255, 255),
				x - radius - view.x, y - 50 - view.y, statbuff);
	}

	int w = img.width, h = img.height;
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;
	if (!seen){
		seen = true;
		show_appear_message(gs->game_chat(), etype());
	}

	if (stats().hp < stats().max_hp)
		gl_draw_statbar(view, x - 10, y - 20, 20, 5, stats().hp,
				stats().max_hp);

	if (stats().hurt_cooldown > 0) {
		float s = 1 - stats().hurt_alpha();
		Colour red(255, 255 * s, 255 * s);
		gl_draw_image(&img, xx - view.x, yy - view.y, red);
	} else {
		gl_draw_image(&img, xx - view.x, yy - view.y);
//		if (gs->solid_test(this)){
//		Colour red(255,0,0);
//		image_display(&img, xx - view.x, yy - view.y,red);
//		}
	}
//	gl_printf(gs->primary_font(), Colour(255,255,255), x - view.x, y-25 -view.y, "id=%d", id);
	//draw_path(gs, eb.path);
}

void EnemyInst::attack(GameState* gs, GameInst* inst, bool ranged) {
	if (stats().has_cooldown())
		return;
	PlayerInst* pinst;
	if ((pinst = dynamic_cast<PlayerInst*>(inst))) {
		if (ranged) {
			Attack& ranged = stats().magicatk;

			Pos p(pinst->x, pinst->y);
			p.x += gs->rng().rand(-12, +13);
			p.y += gs->rng().rand(-12, +13);
			if (gs->tile_radius_test(p.x, p.y, 10)) {
				p.x = pinst->x;
				p.y = pinst->y;
			}
			//	ProjectileInst(sprite_id sprite, obj_id originator, float speed, int range,
			//			int damage, int x, int y, int tx, int ty, bool bounce = false,
			//			int hits = 1, obj_id target = NONE);
			GameInst* bullet = new ProjectileInst(ranged.attack_sprite, id,
					ranged.projectile_speed, ranged.range, ranged.damage, x, y,
					p.x, p.y);
			gs->add_instance(bullet);
			stats().reset_ranged_cooldown();
			stats().cooldown += gs->rng().rand(-4, 5);
		} else {
			if (!gs->game_settings().invincible)
				pinst->stats().hurt(stats().meleeatk.damage);

			char dmgstr[32];
			snprintf(dmgstr, 32, "%d", stats().meleeatk.damage);
			float rx, ry;
			direction_towards(Pos(x, y), Pos(pinst->x, pinst->y), rx, ry, 0.5);
			gs->add_instance(
					new AnimatedInst(pinst->x - 5 + rx * 5, pinst->y + ry * 5,
							-1, 25, rx, ry, dmgstr));

			stats().reset_melee_cooldown();
			stats().cooldown += gs->rng().rand(-4, 5);
		}
	}

}

EnemyInst *EnemyInst::clone() const {
	return new EnemyInst(*this);
}

bool EnemyInst::hurt(GameState* gs, int hp) {
	if (!destroyed && stats().hurt(hp)) {
		gs->add_instance(new AnimatedInst(x, y, etype()->sprite_number, 15));
		gs->monster_controller().deregister_enemy(this);
		gs->remove_instance(this);
		return true;
	}
	return false;
}

void EnemyInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(EnemyInst*) inst = *this;
}
