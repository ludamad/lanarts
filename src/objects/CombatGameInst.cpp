/*
 * CombatGameInst.cpp:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#include <luawrap/luawrap.h>

#include <ldraw/DrawOptions.h>

#include "stats/SpellEntry.h"
#include <lcommon/SerializeBuffer.h>
#include <lcommon/strformat.h>

#include "draw/draw_statbar.h"

#include "draw/SpriteEntry.h"
#include "gamestate/GameState.h"
#include "gamestate/Team.h"
#include "objects/EnemyInst.h"
#include "objects/PlayerInst.h"

#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"

#include "stats/effect_data.h"
#include "stats/stat_formulas.h"
#include "stats/ClassEntry.h"
#include "data/lua_util.h"
#include "stats/items/items.h"

#include <lcommon/math_util.h>
#include <ldraw/colour_constants.h>
#include <draw/fonts.h>

#include "PlayerInst.h"

#include "AnimatedInst.h"
#include "CombatGameInst.h"
#include "ProjectileInst.h"

CombatGameInst::~CombatGameInst() {
    delete field_of_view;
    delete _paths_to_object;
}

static int random_round(MTwist& rng, float f) {
    float rem = fmod(f, 1.0f);
    return int((f - rem) + (rng.genrand_real2() < rem ? 1 : 0));
}

const int HURT_COOLDOWN = 30;
bool CombatGameInst::damage(GameState* gs, float fdmg, CombatGameInst* attacker) {
    if (current_floor == -1) {
        return false; // Don't die twice.
    }
    // Don't damage players during invincibility:
    if (dynamic_cast<PlayerInst*>(this) && gs->game_settings().invincible) {
        return false;
    }
    lua_State* L = gs->luastate();
    auto* map = get_map(gs);
    event_log("CombatGameInst::damage: id %d took %.2f dmg", std::max(0, id), fdmg);

    int dmg = random_round(gs->rng(), fdmg);
    if (dmg == 0) {
        return false; // Do nothing if damage is 0
    }

    if (core_stats().hurt(dmg)) {
        die(gs);
        EnemyInst* e_this = dynamic_cast<EnemyInst*>(this);

        if (attacker != NULL && team != PLAYER_TEAM && e_this) {
            int attacker_xp = attacker->gain_xp_from(gs, this);
            attacker->signal_killed_enemy();
//            PlayerData& pc = gs->player_data();
//            double xpworth = this->xpworth();
//            double n_killed = double(pc.n_enemy_killed(e_this->enemy_type()) - 1) / pc.all_players().size();
//            int kills_before_stale = e_this->etype().kills_before_stale;
//            xpworth *= pow(0.9, n_killed * 25 / kills_before_stale); // sum(0.9**i for i in range(25)) => ~9.28x the monsters xp value over time
//            if (n_killed > kills_before_stale) {
//                xpworth = 0;
//            }
//            float multiplayer_bonus = 1.0f / ((1 + pc.all_players().size()/2.0f) / pc.all_players().size());
//
//            int amnt = round(xpworth * multiplayer_bonus / pc.all_players().size());
//
//            attacker->stats().gain_xp(amnt, attacker);

            char buffstr[32];
            if (attacker_xp != 0) {
                snprintf(buffstr, 32, "%d XP", attacker_xp);
            }
            auto* inst = map->add_instance<AnimatedInst>(
                    gs, ipos(), -1, 25, PosF(), PosF(),
                    AnimatedInst::DEPTH, buffstr, COL_GOLD);
            inst->font = &res::font_bigprimary();
            inst->should_center_font = true;
            return true;
        }
    }

    auto* inst = map->add_instance<AnimatedInst>(
            gs, ipos(), -1, 25, PosF(-1,-1), PosF(), AnimatedInst::DEPTH,
            format("%d", dmg), Colour(255, 148/2, 120/2));
    inst->font = &res::font_bigprimary();
    inst->should_center_font = true;

    if (dynamic_cast<PlayerInst*>(attacker)) {
        // Damage from player causes hurt cooldown
        cooldowns().reset_hurt_cooldown(HURT_COOLDOWN);
        if (dynamic_cast<PlayerInst*>(this)) {
            // Damage from player to player bleeds (causes max_hp loss)
            core_stats().hp_bleed += fdmg;
        }
    }
    return false;
}

static bool enemy_filter(GameInst* g1, GameInst* g2) {
    auto* c1 = dynamic_cast<CombatGameInst*>(g1);
    if (!c1) return false;
    auto* c2 = dynamic_cast<CombatGameInst*>(g2);
    if (!c2) return false;
    return c1->team != c2->team;
}

static bool friendly_player_filter(GameInst* g1, GameInst* g2) {
    auto* c1 = dynamic_cast<CombatGameInst*>(g1);
    if (!c1) return false;
    auto* c2 = dynamic_cast<PlayerInst*>(g2);
    if (!c2) return false;
    return c1->team == c2->team;
}

Pos CombatGameInst::direction_towards_enemy(GameState* gs) {
    return direction_towards_object(gs, enemy_filter);
}

Pos CombatGameInst::direction_towards_ally_player(GameState* gs) {
    return direction_towards_object(gs, friendly_player_filter);
}

static bool specific_inst_filter_data(GameInst* g1, GameInst* g2) {
    static GameInst* expected = NULL;
    if (g1 == NULL) {
        expected = g2;
        return false;
    }
    return g2 == expected;
}

static Pos follow(FloodFillPaths& paths, const Pos& from_xy) {
        FloodFillNode* node = paths.node_at(from_xy);
        return Pos(from_xy.x + node->dx, from_xy.y + node->dy);
}
// TODO find appropriate place for this function
static bool has_visible_monster(GameState* gs, PlayerInst* p = NULL) {
    const std::vector<obj_id>& mids = gs->monster_controller().monster_ids();
    for (int i = 0; i < mids.size(); i++) {
        GameInst* inst = gs->get_instance(mids[i]);
        if (inst && gs->object_visible_test(inst, p)) {
            return true;
        }
    }
    return false;
}


Pos CombatGameInst::direction_towards(GameState* gs, GameInst* obj) {
    specific_inst_filter_data(NULL, obj); // Initialize global state
    int dx = obj->x - x;
    int dy = obj->y - y;
    if (std::max(abs(dx),abs(dy)) < 16 + obj->target_radius + target_radius) {
        if (abs(dx)>0) dx /= abs(dx);
        if (abs(dy)>0) dy /= abs(dy);
        return Pos(dx, dy);
    }
    return direction_towards_object(gs, specific_inst_filter_data);
}


Pos CombatGameInst::direction_towards_object(GameState* gs, col_filterf filter) {
    LANARTS_ASSERT(has_paths_data());
    Pos closest = {-1,-1};
    float min_dist = 10000;//std::numeric_limits<float>::max();
    int dx = 0, dy = 0;
    if (min_dist == 10000) {//std::numeric_limits<float>::max()) {
        FOR_EACH_BBOX(paths_to_object().location(), x, y) {
            auto* node = paths_to_object().node_at({x, y});
            if (node->solid || node->open) {
                continue;
            }
            float dist = node->distance;
            if (dist == 0 && (node->dx != 0 || node->dy != 0)) {
                continue;
            }
            bool is_item = (gs->object_radius_test(this, NULL, 0, filter, (x*32+16), (y*32+16), 12));
            if (is_item && min_dist >= dist) {
                closest = {x,y};
                min_dist = dist;
            }
        }
    }
    if (min_dist != 10000) {//std::numeric_limits<float>::max()) {
        Pos iter = closest;
        Pos next_nearest;
        while (true) {
           Pos next = follow(paths_to_object(), iter);
           auto* next_node = paths_to_object().node_at(next);
           if (next_node->distance == 0) {
               next_nearest = iter;
                break;
           }
           iter = next;
        }
        dx = (next_nearest.x * TILE_SIZE + TILE_SIZE / 2) - x;
        dy = (next_nearest.y * TILE_SIZE + TILE_SIZE / 2) - y;
        if (abs(dx) < effective_stats().movespeed) {
            dx = 0;
        }
        if (abs(dy) < effective_stats().movespeed) {
            dy = 0;
        }
        if (abs(dx) + abs(dy) < effective_stats().movespeed * 2) {
            dx = 0;
            dy = 0;
        }
        if (abs(dx) > 0) dx /= abs(dx);
        if (abs(dy) > 0) dy /= abs(dy);
    }
    return {dx, dy};
}

bool CombatGameInst::damage(GameState* gs, const EffectiveAttackStats& raw_attack, CombatGameInst* attacker, float* final_dmg) {
    lua_push_effectiveattackstats(gs->luastate(), raw_attack);
    LuaStackValue eff_atk_stats(gs->luastate(), -1);
    if (attacker != NULL) {
        effects.for_each([&](Effect& eff) {
            lcall(/*func:*/ eff.entry().raw_lua_object["on_defend_func"], /*args:*/ eff.state, attacker, this, eff_atk_stats);
        });
        attacker->effects.for_each([&](Effect& eff) {
            lcall(/*func:*/ eff.entry().raw_lua_object["on_attack_func"], /*args:*/ eff.state, attacker, this, eff_atk_stats);
        });
    }
    EffectiveAttackStats attack = lua_pop_effectiveattackstats(gs->luastate());

    event_log("CombatGameInst::damage: id %d getting hit by {cooldown = %.2f, "
                      "damage=%.2f, power=%.2f, magic_percentage=%f, physical_percentage=%f}",
              std::max(0, id), attack.cooldown, attack.damage, attack.power,
              attack.magic_percentage,
              attack.physical_percentage());

    float fdmg = damage_formula(attack, effective_stats());
    if (gs->game_settings().verbose_output) {
        char buff[100];
        snprintf(buff, 100, "Attack: [dmg %.2f pow %.2f mag %d%%] -> Damage: %.2f",
                 attack.damage, attack.power, int(attack.magic_percentage * 100),
                 fdmg);
        gs->for_screens([&]() {
            gs->game_chat().add_message(buff);
        });
    }

    if (attack.type_multiplier < 0.60) {
        gs->add_instance(
                new AnimatedInst(ipos() + Pos {10, 0}, -1, 25, PosF(-1, -1), PosF(), AnimatedInst::DEPTH, "Highly ineffective!",
                                 attack.type_multiplier < 1 ? COL_RED : COL_GREEN));
    }
    else if (attack.type_multiplier < 0.85) {
        gs->add_instance(
                new AnimatedInst(ipos() + Pos {10, 0}, -1, 25, PosF(-1, -1), PosF(), AnimatedInst::DEPTH, "Very ineffective!",
                                 attack.type_multiplier < 1 ? COL_RED : COL_GREEN));
    }
    else if (attack.type_multiplier < 0.95) {
        gs->add_instance(
                new AnimatedInst(ipos() + Pos {10, 0}, -1, 25, PosF(-1, -1), PosF(), AnimatedInst::DEPTH, "Ineffective!",
                                 attack.type_multiplier < 1 ? COL_PALE_RED : COL_GREEN));
    } else if (attack.type_multiplier <= 1.05) {
        gs->add_instance(
                new AnimatedInst(ipos() + Pos {10, 0}, -1, 25, PosF(-1, -1), PosF(), AnimatedInst::DEPTH, "Effective!",
                                 COL_PALE_GREEN));
    } else if (attack.type_multiplier <= 1.15) {
        gs->add_instance(
                new AnimatedInst(ipos() + Pos {10, 0}, -1, 25, PosF(-1, -1), PosF(), AnimatedInst::DEPTH, "Quite Effective!",
                                 COL_PALE_GREEN));
    } else if (attack.type_multiplier <= 1.25) {
        gs->add_instance(
                new AnimatedInst(ipos() + Pos {10, 0}, -1, 25, PosF(-1, -1), PosF(), AnimatedInst::DEPTH, "Very Effective!",
                                 attack.type_multiplier < 1 ? COL_RED : COL_GREEN));
    } else {
        gs->add_instance(
                new AnimatedInst(ipos() + Pos {10, 0}, -1, 25, PosF(-1, -1), PosF(), AnimatedInst::DEPTH, "Super Effective!",
                                 attack.type_multiplier < 1 ? COL_RED : COL_GREEN));
    }

    effects.for_each([&](Effect& eff) {
        fdmg = lcall_def(fdmg, /*func:*/ eff.entry().raw_lua_object["on_receive_damage_func"], /*args:*/ eff.state, attacker, this, fdmg);
    });
    attacker->effects.for_each([&](Effect& eff) {
        fdmg = lcall_def(fdmg, /*func:*/ eff.entry().raw_lua_object["on_deal_damage_func"], /*args:*/ eff.state, attacker, this, fdmg);
    });

    return damage(gs, fdmg, attacker);
}

void CombatGameInst::update_field_of_view(GameState* gs) {
}

void CombatGameInst::step(GameState* gs) {
    GameInst::step(gs);
    estats = stats().effective_stats(gs, this);
    stats().step(gs, this, estats);

    // XXX: If we do not sync the new mp & hp values
    // we can get health/mp bars that look more than full
    estats.core.hp = stats().core.hp;
    estats.core.mp = stats().core.mp;
}

/* Getters */
CombatStats& CombatGameInst::stats() {
    return base_stats;
}

EffectiveAttackStats CombatGameInst::effective_atk_stats(MTwist& mt,
        const AttackStats& attack) {
    EffectiveAttackStats eff = effective_stats().with_attack(mt, attack);
    // ITEM IMPL DETAIL
    if (attack.weapon_entry().name == "Magic Sword" && effective_stats().core.mp > 20) {
        eff.damage *= 1.25;
    }
    return eff;
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

void CombatGameInst::draw(GameState *gs, float frame, float alpha) {
    GameInst::draw(gs);
    if (sprite == -1) {
        return;
    }
    GameView& view = gs->view();
    SpriteEntry& spr = game_sprite_data.get(sprite);
    Colour draw_colour = effects.effected_colour();

    if (cooldowns().is_hurting()) {
        float s = 1 - hurt_alpha_value(cooldowns().hurt_cooldown);
        draw_colour = draw_colour.multiply(Colour(255, 255 * s, 255 * s));
    }
    draw_colour.a *= alpha;
    
    int sx = x - spr.width() / 2, sy = y - spr.height() / 2;
    draw_sprite(view, sprite, sx, sy, vx, vy, frame, draw_colour);

    effects.draw_effect_sprites(gs, this, Pos(sx, sy));
    if (is_resting) {
        res::sprite("resting").draw(ldraw::DrawOptions(ldraw::CENTER),
                on_screen(gs, ipos()));
    }
}

void CombatGameInst::post_draw(GameState *gs) {
    SpriteEntry& spr = game_sprite_data.get(sprite);
    GameView& view = gs->view();
    int w = spr.size().w, h = spr.size().h;
    int xx = x - w / 2, yy = y - h / 2;

    if (!view.within_view(xx, yy, w, h))
            return;
    if (!gs->object_visible_test(this))
            return;
    CoreStats& ecore = effective_stats().core;
    //Draw health bar
    int healthbar_offsety = 20;
    if (target_radius > 16)
        healthbar_offsety = target_radius + 8;
    if (ecore.hp < ecore.max_hp) {
        const BBox statbox(x - 10, y - healthbar_offsety, x + 10,
                y - healthbar_offsety + 5);
        draw_statbar(on_screen(gs, statbox), float(ecore.hp) / ecore.max_hp);
        // Draw HP on top of HP bar
        res::font_primary().drawf(ldraw::DrawOptions(ldraw::CENTER, COL_WHITE), on_screen(gs, statbox.center()), "%d", ecore.hp, ecore.max_hp);
    }
    if (dynamic_cast<PlayerInst*>(this) && dynamic_cast<PlayerInst *>(this)->is_focus_player(gs)) {
        res::sprite("spr_effects.good_neutral").draw(on_screen(gs, PosF {x-23, y-23}));
        //res::sprite("spr_effects.good_neutral").draw(on_screen(gs, PosF {x-23, y-23}));
    } else if (dynamic_cast<PlayerInst*>(this)) {
        res::sprite("spr_effects.friendly").draw(on_screen(gs, PosF {x-23, y-23}));
        //res::sprite("spr_effects.good_neutral-2").draw(on_screen(gs, PosF {x-23, y-23}));
    }
}

EffectiveAttackStats lua_hit_callback(LuaValue& callback,
                                      const EffectiveAttackStats& atkstats, GameInst* obj,
                                      GameInst* target);

EffectiveAttackStats lua_attack_stats_callback(lua_State* L, LuaValue& callback,
                                               const EffectiveAttackStats& atkstats, GameInst* obj, GameInst* target);

bool CombatGameInst::melee_attack(GameState* gs, CombatGameInst* inst,
        const Item& weapon, bool ignore_cooldowns, float damage_multiplier) {
    event_log("CombatGameInst::melee_attack: id %d hitting id %d, weapon = id %d", std::max(0, id), inst->id,     weapon.id);

    bool isdead = false;
    if (!ignore_cooldowns && !cooldowns().can_doaction())
        return false;

    MTwist& mt = gs->rng();

    AttackStats attack(weapon);
    EffectiveAttackStats atkstats = effective_atk_stats(mt,
            AttackStats(weapon));
    atkstats.damage *= damage_multiplier;

    // Modify the attack stat function:
    atkstats = lua_attack_stats_callback(gs->luastate(), weapon.weapon_entry().attack_stat_func, atkstats, this, inst);
    atkstats = lua_hit_callback(weapon.weapon_entry().action_func(), atkstats, this, inst);
    if (gs->game_settings().verbose_output) {
        char buff[100];
        snprintf(buff, 100, "Attack: [dmg %.2f pow %.2f mag %d%%]",
                atkstats.damage, atkstats.power, int(atkstats.magic_percentage * 100));
        gs->for_screens([&](){
            gs->game_chat().add_message(buff);
        });

    }

    if (!dynamic_cast<PlayerInst*>(this)) {
        gs->for_screens([&]() {
            if (gs->local_player()->current_floor == current_floor) {
                play("sound/slash.ogg");
            }
        });
    }

    float final_dmg = 0;
    isdead = inst->damage(gs, atkstats, this, &final_dmg);
    lcall(weapon.weapon_entry().attack.on_damage, this, inst, final_dmg);

    effects.for_each([&](Effect& eff) {
        lcall(/*func:*/ eff.entry().on_melee_func, /*args:*/ eff.state, this, inst, final_dmg);
    });
    inst->effects.for_each([&](Effect& eff) {
        lcall(/*func:*/ eff.entry().on_receive_melee_func, /*args:*/ eff.state, this, inst, final_dmg);
    });

    if (!ignore_cooldowns) {
        cooldowns().reset_action_cooldown(
                atkstats.cooldown
                        * estats.cooldown_modifiers.melee_cooldown_multiplier);
    }

    WeaponEntry& wentry = weapon.weapon_entry();
    if (wentry.name != "none") {
        gs->add_instance<AnimatedInst>(inst->ipos(), wentry.attack_sprite(), 25);
    }

    signal_attacked_successfully();
    return isdead;
}

bool CombatGameInst::projectile_attack(GameState* gs, CombatGameInst* inst,
        const Item& weapon, const Item& projectile) {

    if (!cooldowns().can_doaction())
        return false;

    event_log("CombatGameInst::projectile_attack: id %d hitting id %d, weapon = id %d", std::max(0, id), inst ? std::max(inst->id, 0) : 0, weapon.id);
    MTwist& mt = gs->rng();

    WeaponEntry& wentry = weapon.weapon_entry();
    ProjectileEntry& pentry = projectile.projectile_entry();
    AttackStats attack;
    if (!pentry.is_standalone()) {
        attack.weapon = weapon;
    }
    attack.projectile = projectile;
    EffectiveAttackStats atkstats = effective_atk_stats(mt, attack);

    Pos target_p = inst == NULL ? Pos(x + gs->rng().rand(-32, 32), y + gs->rng().rand(-32, 32)) : inst->ipos();
    Pos p = target_p;
    if (dynamic_cast<EnemyInst*>(this) && dynamic_cast<EnemyInst*>(this)->etype().name == "Ogre Mage") {
        p.x += gs->rng().rand(-199, +200);
        p.y += gs->rng().rand(-199, +200);
    } else {
        p.x += gs->rng().rand(-12, +13);
        p.y += gs->rng().rand(-12, +13);
        if (gs->tile_radius_test(p.x, p.y, 10)) {
            p = target_p;
        }
    }

    event_log(
            "CombatGameInst::projectile_attack id=%d created projectile at %d, %d",
            std::max(0, id), p.x, p.y);

    int range = pentry.range();

    bool has_greater_fire = (effects.get_active("AmuletGreaterFire") != NULL);
    bool is_spread_spell = pentry.name == "Mephitize" || pentry.name == "Purple Dragon Projectile";
    if (is_spread_spell || pentry.name == "Trepidize" || (has_greater_fire && pentry.name == "Fire Bolt")) {
        float vx = 0, vy = 0;
        ::direction_towards(Pos{x, y}, p, vx, vy, 10000);
        int directions = (is_spread_spell ? 16 : 4);

        for (int i = 0; i < directions; i++) {
            const float PI = 3.141592;
            float angle = PI / directions * 2 * i;
            Pos new_target{x + cos(angle) * vx - sin(angle) * vy, y + cos(angle) * vy + sin(angle) * vx};
            gs->add_instance<ProjectileInst>(projectile, atkstats, id, ipos(), new_target, pentry.speed,
                                             pentry.range(),
                                             NONE, pentry.can_wall_bounce, pentry.number_of_target_bounces,
                                             pentry.can_pass_through);
        }
    } else {
        gs->add_instance<ProjectileInst>(projectile, atkstats, id, Pos(x, y),
                                         p, pentry.speed, range,
                                         NONE, pentry.can_wall_bounce, pentry.number_of_target_bounces,
                                         pentry.can_pass_through);
    }
    cooldowns().reset_action_cooldown(
        pentry.cooldown() * estats.cooldown_modifiers.ranged_cooldown_multiplier
    );
    cooldowns().action_cooldown += gs->rng().rand(-4, 5);
    return false;
}

bool CombatGameInst::attack(GameState* gs, CombatGameInst* inst,
        const AttackStats& attack) {

    WeaponEntry& weapon = attack.weapon.weapon_entry();

    if (attack.is_ranged()) {
        return projectile_attack(gs, inst, attack.weapon, attack.projectile);
    } else {
        return melee_attack(gs, inst, attack.weapon);
    }
}

void CombatGameInst::init(GameState* gs) {
    estats = stats().effective_stats(gs, this);
    GameInst::init(gs);
    // Make sure current_floor is set before adding to team:
    gs->team_data().add(this);

    if ((dynamic_cast<PlayerInst*>(this)) && !_paths_to_object) {
        _paths_to_object = new FloodFillPaths();
	paths_to_object().initialize(gs->tiles().solidity_map());
    }
    if ((dynamic_cast<PlayerInst*>(this)) && !field_of_view) {
        field_of_view = new fov();
    }
}

void CombatGameInst::deinit(GameState* gs) {
    // Remove before 'deinit' clears current_level:
    gs->team_data().remove(this);
    GameInst::deinit(gs);
}

const float ROUNDING_MULTIPLE = 256.0f;

PosF CombatGameInst::attempt_move_to_position(GameState* gs,
        const PosF& newxy) {

    event_log("CombatGameInst::attempt_move_to_position id=%d, %f, %f", std::max(0, id),
            newxy.x, newxy.y);
    float dx = newxy.x - rx, dy = newxy.y - ry;
    float dist = sqrt(dx * dx + dy * dy);
    dist = round(dist * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;

    bool collided = gs->tile_radius_test(round(newxy.x), round(newxy.y), 20);

    if (!collided) {
        rx = newxy.x, ry = newxy.y;
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

    update_position();

    return Pos(rx, ry);
}

void CombatGameInst::update_position() {
    x = iround(rx); //update based on rounding of true float
    y = iround(ry);
    event_log("Instance id %d integer positions set to (%f,%f) from (%f,%f)", std::max(0, id), x, y, rx, ry);
}

void CombatGameInst::update_position(float newx, float newy) {
    rx = round(newx * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;
    ry = round(newy * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;
    rx = newx, ry = newy;
    event_log("Instance id %d float positions set to (%f,%f)", std::max(0, id), rx, ry);
    update_position();
}

SpellsKnown& CombatGameInst::spells_known() {
    return effective_stats().spells;
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
    combine_hash(hash, core.strength, core.defence);
    combine_hash(hash, core.magic, core.willpower);
    combine_hash(hash, core.hpregen, core.mpregen);
    combine_hash(hash, (unsigned int&) core.hp_regened,
            (unsigned int&) core.mp_regened);
    combine_hash(hash, (unsigned int&) core.hp_regened,
            (unsigned int&) core.spell_velocity_multiplier);

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
    combine_hash(hash, (unsigned int&) vx, (unsigned int&) vy);
    combine_stat_hash(hash, stats());
    return hash;
}

void CombatGameInst::serialize(GameState* gs, SerializeBuffer& serializer) {
    GameInst::serialize(gs, serializer);
    SERIALIZE_POD_REGION(serializer, this, team, current_target);
    base_stats.serialize(gs, serializer);
    estats.serialize(serializer);
}

void CombatGameInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
    GameInst::deserialize(gs, serializer);
    DESERIALIZE_POD_REGION(serializer, this, team, current_target);
    base_stats.deserialize(gs, serializer);
    estats.deserialize(serializer);
    if (team == PLAYER_TEAM && id > 0) {
        _paths_to_object = new FloodFillPaths();
	paths_to_object().initialize(gs->tiles().solidity_map());
        field_of_view = new fov();
        int sx = x / TILE_SIZE;
        int sy = y / TILE_SIZE;
        field_of_view->calculate(gs, vision_radius, sx, sy);
    }
}

ClassStats& CombatGameInst::class_stats() {
    return stats().class_stats;
}

CooldownStats& CombatGameInst::cooldowns() {
    return stats().cooldowns;
}

const float PI = 3.141592f;

void CombatGameInst::use_projectile_spell(GameState* gs, SpellEntry& spl_entry, 
        const Projectile& projectile, const Pos& target) {
    MTwist& mt = gs->rng();
    AttackStats projectile_attack(Weapon(), projectile);
    ProjectileEntry& pentry = projectile.projectile_entry();
    bool wallbounce = pentry.can_wall_bounce, passthrough = pentry.can_pass_through;
    int nbounces = pentry.number_of_target_bounces;
    float speed = pentry.speed * effective_stats().core.spell_velocity_multiplier;

    bool has_greater_fire = effects.has("AmuletGreaterFire") || effects.has("Dragonform");
    bool is_spread_spell = pentry.name == "Mephitize" || pentry.name == "Purple Dragon Projectile";
    if (is_spread_spell || pentry.name == "Trepidize" || (has_greater_fire && pentry.name == "Fire Bolt") || (pentry.name == "Tornado Storm")) {
        float vx = 0, vy = 0;
        ::direction_towards(Pos {x, y}, target, vx, vy, 10000);
        int directions = (pentry.name == "Trepidize" ? 4 : 16);
        if (pentry.name == "Fire Bolt") directions = 4;
        if (pentry.name == "Tornado Storm") directions = 8;

        for (int i = 0; i < directions; i++) {
            float angle = PI / directions * 2 * i;
            Pos new_target {x + cos(angle) * vx - sin(angle) * vy, y + cos(angle) * vy + sin(angle) * vx};
            GameInst* pinst = new ProjectileInst(projectile,
                    effective_atk_stats(mt, projectile_attack), id,
                    Pos(x, y), new_target, speed, pentry.range(), NONE,
                    wallbounce, nbounces, passthrough);
            gs->add_instance(pinst);
        }

    } else {
        gs->add_instance<ProjectileInst>(projectile,
                                         effective_atk_stats(mt, projectile_attack), id,
                                         Pos(x, y), target, speed, pentry.range(), NONE,
                                         wallbounce, nbounces, passthrough);
    }
}

void CombatGameInst::use_spell(GameState* gs, SpellEntry& spl_entry, const Pos& target, GameInst* target_object) {
    lua_State* L = gs->luastate();

    use_mp(gs, spl_entry.mp_cost);
    float spell_cooldown_mult =
            effective_stats().cooldown_modifiers.spell_cooldown_multiplier;
    cooldowns().reset_action_cooldown(
            spl_entry.cooldown * spell_cooldown_mult);
    float cooldown_mult = game_spell_data.call_lua(spl_entry.name, "cooldown_multiplier", /*Default value:*/ 1.0f, this);
    spell_cooldown_mult *= cooldown_mult;
//    game_spell_data.
    // Set global cooldown for spell:
    cooldowns().spell_cooldowns[spl_entry.id] = (int)std::max(spl_entry.spell_cooldown * spell_cooldown_mult, cooldowns().spell_cooldowns[spl_entry.id] * spell_cooldown_mult);
    if (spl_entry.uses_projectile()) {
        use_projectile_spell(gs, spl_entry, spl_entry.projectile, target);
    } else {
        // Use action_func callback
        lcall(spl_entry.action_func, /*caster*/ this, target.x, target.y, /*target object*/ target_object);
    }
}

void CombatGameInst::try_use_spell(GameState* gs, SpellEntry& spl_entry, const Pos& target, GameInst* target_object) {
    if (!effective_stats().allowed_actions.can_use_spells) {
        return;
    }

    if (cooldowns().spell_cooldowns[spl_entry.id] > 0) {
        return;
    }
    if (spl_entry.mp_cost > core_stats().mp
        || (!spl_entry.can_cast_with_cooldown && !cooldowns().can_doaction())) {
        return;
    }

    use_spell(gs, spl_entry, target, target_object);

    cooldowns().action_cooldown *= effective_stats().cooldown_mult;
    cooldowns().reset_rest_cooldown(REST_COOLDOWN);
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

static int players_gain_xp(GameState* gs, team_id team, float xp, bool use_bonus) {
    PlayerData& pd = gs->player_data();
    std::vector<PlayerDataEntry>& players = pd.all_players();

    int n_players = 0;
    for (auto& player : players) {
        if (player.player()->team == team) {
            n_players++;
        }
    }
    if (n_players == 0) {
        return 0;
    }
    float multiplayer_bonus = 1.0f / ((1 + n_players/2.0f) / n_players);
    if (!use_bonus) {
        multiplayer_bonus = 1;
    }
    xp *= multiplayer_bonus;
    auto xp_portion = (int)ceil(xp * multiplayer_bonus / n_players);

    for (auto& player : players) {
        auto* inst = player.player();
        if (inst->team == team) {
            inst->gain_xp(gs, xp_portion);
        }
    }
    return xp;
}

int CombatGameInst::team_gain_xp(GameState* gs, float xp, bool use_bonus) {
    return players_gain_xp(gs, team, xp, use_bonus);
}

int CombatGameInst::gain_xp_from(GameState* gs, CombatGameInst* inst) {
    PlayerData& pc = gs->player_data();
    signal_killed_enemy();
    // Nothing for killing players yet:
    if (!dynamic_cast<EnemyInst*>(inst)) {
        return 0;
    }

    double xpworth = ((EnemyInst*)inst)->xpworth();
    // TODO track kills per team
    double n_killed = (pc.n_enemy_killed(((EnemyInst*) inst)->enemy_type()) - 1) / pc.all_players().size();
    // Level out at 25% of the original XP, after ~27 kills per player
    xpworth *= std::max(0.25, pow(0.9, n_killed));
    return team_gain_xp(gs, xpworth, /*use bonus*/ true);
}

void CombatGameInst::use_mp(GameState* gs, int mp) {
    if (mp == 0) {
        // No need to call effect methods
        return;
    }
    core_stats().mp -= mp;
    effects.for_each([&](Effect& eff) {
        lcall(eff.method("on_use_mp"), /*args:*/ eff.state, this, mp);
    });
}
