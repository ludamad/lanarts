/*
 * CombatGameInst.cpp:
 *  Represents an instance that is affected by combat, ie enemies and players
 */

#include <luawrap/luawrap.h>

#include <ldraw/DrawOptions.h>

#include <lcommon/SerializeBuffer.h>
#include <lcommon/strformat.h>

#include "draw/draw_statbar.h"

#include "draw/SpriteEntry.h"
#include "gamestate/GameState.h"
#include "gamestate/Team.h"
#include "objects/EnemyInst.h"

#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"

#include "stats/effect_data.h"
#include "stats/stat_formulas.h"
#include "stats/ClassEntry.h"

#include <lcommon/math_util.h>

#include "PlayerInst.h"

#include "AnimatedInst.h"
#include "CombatGameInst.h"
#include "ProjectileInst.h"

CombatGameInst::~CombatGameInst() {
    delete field_of_view;
    delete _paths_to_object;
}

const int HURT_COOLDOWN = 30;
bool CombatGameInst::damage(GameState* gs, int dmg) {
    if (current_floor == -1) {
        return false; // Don't die twice.
    }
    lua_State* L = gs->luastate();
    auto* prev_level = gs->get_level();
    gs->set_level(gs->get_level(current_floor));
    event_log("CombatGameInst::damage: id %d took %d dmg\n", id, dmg);

    for (Effect& eff : effects.effects) {
        EffectEntry& entry = game_effect_data.get(eff.id);
        if (!eff.is_active() || entry.on_damage_func.isnil()) {
            continue;
        }
        entry.on_damage_func.push();
        luawrap::push(L, eff.state);
        luawrap::push(L, this);
        luawrap::push(L, dmg);
        lua_call(L, 3, 1);
        if (!lua_isnil(L, -1)) {
            dmg = lua_tonumber(L, -1);
        }
        lua_pop(L, 1);
    }

    if (core_stats().hurt(dmg)) {
        die(gs);
        gs->set_level(prev_level);
        return true;
    }

    cooldowns().reset_hurt_cooldown(HURT_COOLDOWN);
    gs->set_level(prev_level);
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


bool CombatGameInst::damage(GameState* gs, const EffectiveAttackStats& attack) {
    event_log("CombatGameInst::damage: id %d getting hit by {cooldown = %d, "
            "damage=%d, power=%d, magic_percentage=%f, physical_percentage=%f}",
            id, attack.cooldown, attack.damage, attack.power,
            attack.magic_percentage, 
            attack.physical_percentage());

    float fdmg = damage_formula(attack, effective_stats());
    if (fdmg < 1 && gs->rng().randf() < fdmg) {
        fdmg = 1;
    }
    int dmg = fdmg;

    if (gs->game_settings().verbose_output) {
        char buff[100];
        snprintf(buff, 100, "Attack: [dmg %d pow %d mag %d%%] -> Damage: %d",
                attack.damage, attack.power, int(attack.magic_percentage * 100),
                dmg);
        gs->for_screens( [&] () {
            gs->game_chat().add_message(buff);
        });

    }

    gs->add_instance(
            new AnimatedInst(ipos(), -1, 25, PosF(-1,-1), PosF(), AnimatedInst::DEPTH,
                    format("%d", dmg), Colour(255, 148, 120)));

    return damage(gs, dmg);
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
    }
    if (dynamic_cast<PlayerInst*>(this) && dynamic_cast<PlayerInst *>(this)->is_focus_player(gs)) {
        res::sprite("spr_enemies.good_neutral").draw(on_screen(gs, PosF {x-16, y-16}));
        res::sprite("spr_enemies.good_neutral").draw(on_screen(gs, PosF {x-16, y-16}));
    }
}

bool CombatGameInst::melee_attack(GameState* gs, CombatGameInst* inst,
        const Item& weapon, bool ignore_cooldowns, float damage_multiplier) {
    event_log("CombatGameInst::melee_attack: id %d hitting id %d, weapon = id %d\n", id, inst->id,     weapon.id);

    bool isdead = false;
    if (!ignore_cooldowns && !cooldowns().can_doaction())
        return false;

    MTwist& mt = gs->rng();

    AttackStats attack(weapon);
    EffectiveAttackStats atkstats = effective_atk_stats(mt,
            AttackStats(weapon));

    float damage = damage_formula(atkstats, inst->effective_stats()) * damage_multiplier;

    if (gs->game_settings().verbose_output) {
        char buff[100];
        snprintf(buff, 100, "Attack: [dmg %d pow %d mag %d%%] -> Damage: %f",
                atkstats.damage, atkstats.power, int(atkstats.magic_percentage * 100),
                damage);
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

    // Callbacks on attacker object:
    for (Effect& eff : effects.effects) {
        if (!eff.is_active()) {
            continue;
        }
        auto& entry = game_effect_data.get(eff.id);
        if (!entry.on_melee_func.isnil()) {
            entry.on_melee_func.push();
            lua_State* L = gs->luastate();
            luawrap::push(L, eff.state);
            luawrap::push(L, this);
            luawrap::push(L, inst);
            luawrap::push(L, damage);
            lua_push_effectiveattackstats(L, atkstats);
            lua_call(L, 5, 1);
            if (!lua_isnil(L, -1)) {
                damage = lua_tonumber(L, -1);
            }
            lua_pop(L, 1);
        }
    }
    
    // Callbacks on attacked object:
    for (Effect& eff : inst->effects.effects) {
        if (!eff.is_active()) {
            continue;
        }
        auto& entry = game_effect_data.get(eff.id);
        if (!entry.on_receive_melee_func.isnil()) {
            entry.on_receive_melee_func.push();
            lua_State* L = gs->luastate();
            luawrap::push(L, eff.state);
            luawrap::push(L, this);
            luawrap::push(L, inst);
            luawrap::push(L, damage);
            lua_push_effectiveattackstats(L, atkstats);
            lua_call(L, 5, 1);
            if (!lua_isnil(L, -1)) {
                damage = lua_tonumber(L, -1);
            }
            lua_pop(L, 1);
        }
    }


    // Don't damage players during invincibility:
    if (dynamic_cast<PlayerInst*>(this) || !gs->game_settings().invincible) {
        isdead = inst->damage(gs, int(damage));
    }

    char dmgstr[32];
    snprintf(dmgstr, 32, "%d", int(damage));
    float rx, ry;
    ::direction_towards(Pos(x, y), Pos(inst->x, inst->y), rx, ry, 0.5);
    rx = round(rx * 256.0f) / 256.0f;
    ry = round(ry * 256.0f) / 256.0f;
    gs->add_instance(
            new AnimatedInst(Pos(inst->x - 5 + rx * 5, inst->y + ry * 5), -1,
                    25, PosF(rx, ry), PosF(), AnimatedInst::DEPTH, dmgstr,
                    Colour(255, 148, 120)));

    if (!ignore_cooldowns) {
        cooldowns().reset_action_cooldown(
                atkstats.cooldown
                        * estats.cooldown_modifiers.melee_cooldown_multiplier);
    }

    WeaponEntry& wentry = weapon.weapon_entry();
    if (wentry.name != "none") {
        gs->add_instance(
                new AnimatedInst(inst->ipos(), wentry.attack_sprite(), 25));
    }

    signal_attacked_successfully();
    if (isdead && dynamic_cast<EnemyInst*>(inst) && inst->team == MONSTER_TEAM) {
            PlayerData& pc = gs->player_data();
            signal_killed_enemy();

            char buffstr[32];
            double xpworth = ((EnemyInst*)inst)->xpworth();
            double n_killed = (pc.n_enemy_killed(((EnemyInst*) inst)->enemy_type()) - 1) / pc.all_players().size();
            int kills_before_stale = ((EnemyInst*) inst)->etype().kills_before_stale;
            xpworth *= pow(0.9, n_killed * 25 / kills_before_stale); // sum(0.9**i for i in range(25)) => ~9.28x the monsters xp value over time
            if (n_killed > kills_before_stale) {
                xpworth = 0;
            }
            // Compensates for enemy numbers not-quite scaling:
            float multiplayer_bonus = 1.0f / ((1 + pc.all_players().size()/2.0f) / pc.all_players().size());
            int amnt = round(xpworth * multiplayer_bonus / pc.all_players().size());

            players_gain_xp(gs, amnt);
            if (xpworth == 0) {
                snprintf(buffstr, 32, "STALE", amnt);
            } else {
                snprintf(buffstr, 32, "%d XP", amnt);
            }
            gs->add_instance(
                    new AnimatedInst(Pos(inst->x - 5, inst->y - 5), -1, 25,
                            PosF(), PosF(), AnimatedInst::DEPTH, buffstr,
                            Colour(255, 215, 11)));
            return true;
        }
    return isdead;
}

bool CombatGameInst::projectile_attack(GameState* gs, CombatGameInst* inst,
        const Item& weapon, const Item& projectile) {

    if (!cooldowns().can_doaction())
        return false;

    event_log("CombatGameInst::projectile_attack: id %d hitting id %d, weapon = id %d\n", id, inst ? inst->id : 0, weapon.id);
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
            "CombatGameInst::projectile_attack id=%d created projectile at %d, %d\n",
            id, p.x, p.y);

    int range = pentry.range();

    bool has_greater_fire = (effects.get_active("AmuletGreaterFire") != NULL);
    bool is_spread_spell = pentry.name == "Mephitize" || pentry.name == "Purple Dragon Projectile";
    if (is_spread_spell || pentry.name == "Trepidize" || (has_greater_fire && pentry.name == "Fire Bolt")) {
          float vx = 0, vy = 0;
          ::direction_towards(Pos {x, y}, p, vx, vy, 10000);
          int directions = (is_spread_spell ? 16 : 4);

          for (int i = 0; i < directions; i++) {
              const float PI =3.141592;
              float angle = PI / directions * 2 * i;
              Pos new_target {x + cos(angle) * vx - sin(angle) * vy, y + cos(angle) * vy + sin(angle) * vx};
              GameInst* pinst = new ProjectileInst(projectile, atkstats, id, ipos(), new_target, pentry.speed, pentry.range(), 
                NONE, pentry.can_wall_bounce, pentry.number_of_target_bounces, pentry.can_pass_through);
              gs->add_instance(pinst);
          }
      } else {
          GameInst* bullet = new ProjectileInst(projectile, atkstats, id, Pos(x, y),
            p, pentry.speed, range,
            NONE, pentry.can_wall_bounce, pentry.number_of_target_bounces, pentry.can_pass_through);
          gs->add_instance(bullet);
    }
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
    estats = stats().effective_stats(gs, this);
    GameInst::init(gs);
    // Make sure current_floor is set before adding to team:
    gs->team_data().add(this);

    if (team == PLAYER_TEAM && !_paths_to_object) {
        _paths_to_object = new FloodFillPaths();
	paths_to_object().initialize(gs->tiles().solidity_map());
    }
    if (team == PLAYER_TEAM && !field_of_view) {
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

    event_log("CombatGameInst::attempt_move_to_position id=%d, %f, %f\n", id,
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
    event_log("Instance id %d integer positions set to (%f,%f) from (%f,%f)\n", id, x, y, rx, ry);
}

void CombatGameInst::update_position(float newx, float newy) {
    rx = round(newx * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;
    ry = round(newy * ROUNDING_MULTIPLE) / ROUNDING_MULTIPLE;
    rx = newx, ry = newy;
    event_log("Instance id %d float positions set to (%f,%f)\n", id, rx, ry);
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
    if (team == PLAYER_TEAM) {
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

void CombatGameInst::gain_xp_from(GameState* gs, CombatGameInst* inst, float dx,
        float dy) {
    if (team == MONSTER_TEAM) {
        return;
    }
    PlayerData& pc = gs->player_data();
    signal_killed_enemy();
    // Nothing for killing players yet:
    if (!dynamic_cast<EnemyInst*>(inst)) {
        return;
    }

    char buffstr[32];
    double xpworth = ((EnemyInst*)inst)->xpworth();
    double n_killed = (pc.n_enemy_killed(((EnemyInst*) inst)->enemy_type()) - 1) / pc.all_players().size();
    int kills_before_stale = ((EnemyInst*) inst)->etype().kills_before_stale;
    xpworth *= pow(0.9, n_killed * 25 / kills_before_stale); // sum(0.9**i for i in range(25)) => ~9.28x the monsters xp value over time
    if (n_killed > kills_before_stale) {
        xpworth = 0;
    }
    float multiplayer_bonus = 1.0f / ((1 + pc.all_players().size()/2.0f) / pc.all_players().size());

    int amnt = round(xpworth * multiplayer_bonus / pc.all_players().size());

    players_gain_xp(gs, amnt);
    if (xpworth == 0) {
        snprintf(buffstr, 32, "STALE", amnt);
    } else {
        snprintf(buffstr, 32, "%d XP", amnt);
    }
    gs->add_instance(
            new AnimatedInst(Pos(inst->x + dx, inst->y + dy), -1, 25,
                    PosF(), PosF(), AnimatedInst::DEPTH, buffstr,
                    Colour(255, 215, 11)));
}
