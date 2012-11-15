/*
 * CombatGameInst.cpp:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#include "../display/sprite_data.h"
#include "../gamestate/GameState.h"

#include <common/SerializeBuffer.h>

#include "../stats/items/ProjectileEntry.h"
#include "../stats/items/WeaponEntry.h"

#include "../stats/effect_data.h"
#include "../stats/stat_formulas.h"

#include "../util/math_util.h"

#include "player/PlayerInst.h"

#include "AnimatedInst.h"
#include "CombatGameInst.h"
#include "ProjectileInst.h"

const int HURT_COOLDOWN = 30;
bool CombatGameInst::damage(GameState* gs, int dmg) {
	if (core_stats().hurt(dmg)) {
		die(gs);
		return true;
	}
	signal_was_damaged();
	cooldowns().reset_hurt_cooldown(HURT_COOLDOWN);
	return false;
}
bool CombatGameInst::damage(GameState* gs, const EffectiveAttackStats& attack) {

	int dmg = damage_formula(attack, effective_stats());

	if (gs->game_settings().verbose_output) {
		char buff[100];
		snprintf(buff, 100, "Attack: [dmg %d pow %d mag %d%%] -> Damage: %d",
				attack.damage, attack.power, int(attack.magic_percentage * 100),
				dmg);
		gs->game_chat().add_message(buff);

	}

	char dmgstr[32];
	snprintf(dmgstr, 32, "%d", dmg);
	gs->add_instance(
			new AnimatedInst(Pos(), -1, 25, Posf(), Posf(), AnimatedInst::DEPTH,
					dmgstr, Colour(255, 148, 120)));

	return damage(gs, dmg);
}

void CombatGameInst::update_field_of_view(GameState* gs) {
}

void CombatGameInst::step(GameState* gs) {
	estats = stats().effective_stats(gs, this);
	stats().step(gs, this, estats);
	// XXX: If we do not sync the new mp & hp values
	// we can get drawing artifacts
	estats.core.hp = stats().core.hp;
	estats.core.mp = stats().core.mp;
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
	GameView& view = gs->view();
	SpriteEntry& spr = game_sprite_data[sprite];
	Colour draw_colour = effects().effected_colour();

	if (cooldowns().is_hurting()) {
		float s = 1 - hurt_alpha_value(cooldowns().hurt_cooldown);
		draw_colour = draw_colour.multiply(Colour(255, 255 * s, 255 * s));
	}

	int sx = x - spr.width() / 2, sy = y - spr.height() / 2;
	gl_draw_sprite(view, sprite, sx, sy, vx, vy, gs->frame(), draw_colour);

	effects().draw_effect_sprites(gs, Pos(sx, sy));

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
		const BBox statbox(x - 10, y - healthbar_offsety, x + 10,
				y - healthbar_offsety + 5);
		gl_draw_statbar(view, statbox, ecore.hp, ecore.max_hp);
	}
}

bool CombatGameInst::melee_attack(GameState* gs, CombatGameInst* inst,
		const Item& weapon) {
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
			new AnimatedInst(Pos(inst->x - 5 + rx * 5, inst->y + ry * 5), -1,
					25, Posf(rx, ry), Posf(), AnimatedInst::DEPTH, dmgstr,
					Colour(255, 148, 120)));

	cooldowns().reset_action_cooldown(
			atkstats.cooldown
					* estats.cooldown_modifiers.melee_cooldown_multiplier);

	WeaponEntry& wentry = weapon.weapon_entry();
	if (wentry.name != "none") {
		gs->add_instance(
				new AnimatedInst(inst->pos(), wentry.attack_sprite(), 25));
	}

	signal_attacked_successfully();

	return isdead;
}

bool CombatGameInst::projectile_attack(GameState* gs, CombatGameInst* inst,
		const Item& weapon, const Item& projectile) {
	if (!cooldowns().can_doaction())
		return false;
	MTwist& mt = gs->rng();

	WeaponEntry& wentry = weapon.weapon_entry();
	ProjectileEntry& pentry = projectile.projectile_entry();
	AttackStats attack;
	if (!pentry.is_standalone()) {
		attack.weapon = weapon;
	}
	attack.projectile = projectile;
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
			p, pentry.speed, pentry.range());
	gs->add_instance(bullet);
	cooldowns().reset_action_cooldown(
			pentry.cooldown()
					* estats.cooldown_modifiers.ranged_cooldown_multiplier);
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

void CombatGameInst::init(GameState* gs) {
	GameInst::init(gs);
	estats = stats().effective_stats(gs, this);
}

static float proportion_in_same_dir(float vx1, float vy1, float vx2,
		float vy2) {
	float dir1 = atan2(vy1, vx1);
	float dir2 = atan2(vy2, vx2);

	return cos(dir2 - dir1);
}

bool in_corridor_heurestic(GameState* gs, const Pos& p, float vx, float vy) {
	bool in_xy = abs(vx) > abs(vy);

	GameTiles& tiles = gs->tiles();
	int solid = 0;
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			if (tiles.is_solid(p.x + dx, p.y + dy)) {
				solid++;
			}
		}
	}
	return solid >= 4;
}

const float ROUNDING_MULTIPLE = 256.0f;

void CombatGameInst::attempt_move_to_position(GameState* gs, float& newx,
		float& newy) {

	float dx = newx - rx, dy = newy - ry;
	float dist = sqrt(dx * dx + dy * dy);

	bool collided = gs->tile_radius_test(round(newx), round(newy), 20);

	if (!collided) {
		rx = newx, ry = newy;
	} else {
		float nx = round(rx + vx), ny = round(ry + vy);
		bool collided = gs->tile_radius_test(nx, ny, radius);
		if (collided) {
			bool hitsx = gs->tile_radius_test(nx, y, radius);
			bool hitsy = gs->tile_radius_test(x, ny, radius);
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
	newx = rx, newy = ry;

	update_position();
}

void CombatGameInst::update_position() {
	x = (int)round(rx); //update based on rounding of true float
	y = (int)round(ry);
	event_log("Instance id %d integer positions set to (%d,%d)\n", id, x, y);
}

void CombatGameInst::update_position(float newx, float newy) {
	rx = round(newx * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;
	ry = round(newy * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;
	rx = newx, ry = newy;
	event_log("Instance id %d float positions set to (%f,%f)\n", id, rx, ry);
	update_position();
}

SpellsKnown& CombatGameInst::spells_known() {
	return stats().spells;
}
static void combine_hash(unsigned int& hash, unsigned int val1, unsigned val2) {
	hash ^= (hash >> 11) * val1;
	hash ^= val1;
	hash ^= (hash >> 11) * val2;
	hash ^= val2;
	hash ^= hash << 11;
}
static void combine_stat_hash(unsigned int& hash, CombatStats& stats) {
	ClassStats& cstats = stats.class_stats;
	CoreStats& core = stats.core;
	Inventory& inventory = stats.equipment.inventory;

	combine_hash(hash, core.hp, core.max_hp);
	combine_hash(hash, core.mp, core.max_mp);
	combine_hash(hash, cstats.xp, cstats.classid);
	for (int i = 0; i < inventory.max_size(); i++) {
		if (inventory.slot_filled(i)) {
			ItemSlot& itemslot = inventory.get(i);
			combine_hash(hash, itemslot.amount(), itemslot.id());
		}
	}
}
unsigned int CombatGameInst::integrity_hash() {
	unsigned int hash = GameInst::integrity_hash();
	combine_hash(hash, (unsigned int&)vx, (unsigned int&)vy);
	combine_stat_hash(hash, stats());
	return hash;
}

void CombatGameInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::serialize(gs, serializer);
	SERIALIZE_POD_REGION(serializer, this, vx, kills);
	base_stats.serialize(gs, serializer);
	serializer.write(estats);
}

void CombatGameInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::deserialize(gs, serializer);
	DESERIALIZE_POD_REGION(serializer, this, vx, kills);
	base_stats.deserialize(gs, serializer);
	serializer.read(estats);
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

EquipmentStats& CombatGameInst::equipment() {
	return stats().equipment;
}

simul_id& CombatGameInst::collision_simulation_id() {
	return simulation_id;
}

