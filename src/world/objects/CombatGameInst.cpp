/*
 * CombatGameInst.cpp:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#include "../../data/sprite_data.h"
#include "../../data/effect_data.h"
#include "../../data/weapon_data.h"

#include "../../gamestats/stat_formulas.h"

#include "../../util/math_util.h"

#include "../GameState.h"

#include "../utility_objects/AnimatedInst.h"

#include "CombatGameInst.h"
#include "ProjectileInst.h"
#include "PlayerInst.h"

const int HURT_COOLDOWN = 30;
bool CombatGameInst::damage(GameState* gs, int dmg) {
	if (core_stats().hurt(dmg)) {
		die(gs);
		return true;
	}
	cooldowns().reset_hurt_cooldown(HURT_COOLDOWN);
	return false;
}

void CombatGameInst::update_field_of_view() {
}

void CombatGameInst::step(GameState* gs) {
	estats = stats().effective_stats_without_atk(gs);
	stats().step();
}

/* Getters */
CombatStats& CombatGameInst::stats() {
	return base_stats;
}

EffectiveAttackStats CombatGameInst::effective_atk_stats(MTwist& mt,
		const AttackStats& attack) {
	return effective_stats().with_attack(mt, attack);
}

EffectiveStats& CombatGameInst::effective_stats() {
	return estats;
}

static float hurt_alpha_value(int hurt_cooldown) {
	if (hurt_cooldown < HURT_COOLDOWN / 2)
		return float(hurt_cooldown) / HURT_COOLDOWN / 2 * 0.7f + 0.3f;
	else
		return (HURT_COOLDOWN - hurt_cooldown) / 10 * 0.7f + 0.3f;
}

void CombatGameInst::draw(GameState *gs) {
	GameView& view = gs->window_view();
	SpriteEntry& spr = game_sprite_data[spriteid];
	Colour draw_colour(255, 255, 255);

	int haste_effect = get_effect_by_name("Haste");
	if (effects().get(haste_effect)) {
		Effect* e = effects().get(haste_effect);
		float s = e->t_remaining / 200.0;
		if (s > 1)
			s = 1;
		draw_colour = Colour(255 * (1 - s), 255 * (1 - s), 255);
	} else if (cooldowns().is_hurting()) {
		float s = 1 - hurt_alpha_value(cooldowns().hurt_cooldown);
		draw_colour = Colour(255, 255 * s, 255 * s);
	}
	gl_draw_sprite_entry(view, spr, x - spr.width() / 2, y - spr.height() / 2,
			vx, vy, gs->frame(), draw_colour);

	if (is_resting) {
		GLimage& restimg =
				game_sprite_data[get_sprite_by_name("resting")].img();
		gl_draw_image(view, restimg, x - spr.width() / 2, y - spr.height() / 2);
	}
	CoreStats& ecore = effective_stats().core;

	//Draw health bar
	int healthbar_offsety = 20;
	if (target_radius > 16)
		healthbar_offsety = target_radius + 8;
	if (ecore.hp < ecore.max_hp) {
		gl_draw_statbar(view, x - 10, y - healthbar_offsety, 20, 5, ecore.hp,
				ecore.max_hp);
	}
}

bool CombatGameInst::melee_attack(GameState* gs, CombatGameInst* inst,
		const Weapon& weapon) {
	bool isdead = false;
	if (!cooldowns().can_doaction())
		return false;

	MTwist& mt = gs->rng();

	AttackStats attack(weapon);
	EffectiveAttackStats atkstats = effective_atk_stats(mt,
			AttackStats(weapon));

	int damage = damage_formula(atkstats, inst->effective_stats());

	if (dynamic_cast<PlayerInst*>(this) || !gs->game_settings().invincible) {
		isdead = inst->damage(gs, damage);
	}

	char dmgstr[32];
	snprintf(dmgstr, 32, "%d", damage);
	float rx, ry;
	direction_towards(Pos(x, y), Pos(inst->x, inst->y), rx, ry, 0.5);
	gs->add_instance(
			new AnimatedInst(inst->x - 5 + rx * 5, inst->y + ry * 5, -1, 25, rx,
					ry, AnimatedInst::DEPTH, dmgstr, Colour(255, 148, 120)));

	cooldowns().reset_action_cooldown(atkstats.cooldown);
	cooldowns().action_cooldown += gs->rng().rand(-4, 5);

	WeaponEntry wentry = weapon.weapon_entry();
	if (wentry.name != "none") {
		gs->add_instance(
				new AnimatedInst(inst->x, inst->y, wentry.attack_sprite, 25));
	}

	return isdead;
}

bool CombatGameInst::projectile_attack(GameState* gs, CombatGameInst* inst,
		const Weapon& weapon, const Projectile& projectile) {
	if (!cooldowns().can_doaction())
		return false;
	MTwist& mt = gs->rng();

	WeaponEntry& wentry = weapon.weapon_entry();
	ProjectileEntry& pentry = projectile.projectile_entry();
	EffectiveAttackStats atkstats = effective_atk_stats(mt,
			AttackStats(weapon, projectile));

	Pos p(inst->x, inst->y);
	p.x += gs->rng().rand(-12, +13);
	p.y += gs->rng().rand(-12, +13);
	if (gs->tile_radius_test(p.x, p.y, 10)) {
		p.x = inst->x;
		p.y = inst->y;
	}

	GameInst* bullet = new ProjectileInst(projectile, atkstats, id, Pos(x, y),
			p, pentry.speed, pentry.range);
	gs->add_instance(bullet);
	cooldowns().reset_action_cooldown(pentry.cooldown);
	cooldowns().action_cooldown += gs->rng().rand(-4, 5);
	return false;
}

bool CombatGameInst::attack(GameState* gs, CombatGameInst* inst,
		const AttackStats& attack) {

	if (attack.is_ranged()) {
		return projectile_attack(gs, inst, attack.weapon, attack.projectile);
	} else {
		return melee_attack(gs, inst, attack.weapon);
	}
}

void CombatGameInst::equip(item_id item, int amnt) {
	equipment().equip(item, amnt);
}

void CombatGameInst::init(GameState* gs) {
	estats = stats().effective_stats_without_atk(gs);
}

void CombatGameInst::attempt_move_to_position(GameState *gs, float& newx, float& newy) {
	const float ROUNDING_MULTIPLE = 65536.0f;

	float dx = newx - rx, dy = newy - ry;
	float dist = sqrt(dx * dx + dy * dy);

	bool collided = gs->tile_radius_test(round(newx), round(newy), /*radius+4*/
	20);

	if (!collided) {
		rx = newx, ry = newy;
	} else {
		float nx = round(rx + vx), ny = round(ry + vy);
		bool collided = gs->tile_radius_test(nx, ny, radius);
		if (collided) {
			bool hitsx = gs->tile_radius_test(nx, y, radius, true, -1);
			bool hitsy = gs->tile_radius_test(x, ny, radius, true, -1);
			if (hitsy || hitsx || collided) {
				if (hitsx) {
					vx = 0;
				}
				if (hitsy) {
					vy = 0;
				}
				if (!hitsy && !hitsx) {
					vx = -vx;
					vy = -vy;
				}
			}
		}
		vx = round(vx * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;
		vy = round(vy * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;
		rx += vx;
		ry += vy;
	}

	rx = round(rx * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;
	ry = round(ry * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;

	newx = rx, newy = ry;

	update_position();
}

void CombatGameInst::update_position() {
	x = (int) round(rx); //update based on rounding of true float
	y = (int) round(ry);
}

void CombatGameInst::update_position(float newx, float newy) {
	rx = newx, ry = newy;
	update_position();
}

team_id& CombatGameInst::team() {
	return teamid;
}

ClassStats& CombatGameInst::class_stats() {
	return stats().class_stats;
}

CooldownStats& CombatGameInst::cooldowns() {
	return stats().cooldowns;
}

EffectStats& CombatGameInst::effects() {
	return stats().effects;
}

CoreStats& CombatGameInst::core_stats() {
	return stats().core;
}

Inventory& CombatGameInst::inventory() {
	return stats().equipment.inventory;
}

Equipment& CombatGameInst::equipment() {
	return stats().equipment;
}
