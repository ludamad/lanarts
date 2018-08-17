/*
 * PlayerInstAttackActions.cpp
 *  Implements the various attack and misc spell actions the player can perform.
 */

#include <lua.hpp>

#include <luawrap/luawrap.h>

#include "data/game_data.h"
#include "data/lua_util.h"
#include "draw/colour_constants.h"
#include "draw/draw_sprite.h"
#include "draw/SpriteEntry.h"
#include "draw/TileEntry.h"
#include "gamestate/GameState.h"

#include "lua_api/lua_api.h"

#include "stats/items/ItemEntry.h"
#include "stats/SpellEntry.h"

#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"

#include "stats/SpellEntry.h"

#include <lcommon/math_util.h>

#include "lanarts_defines.h"

#include "objects/EnemyInst.h"

#include "objects/ProjectileInst.h"
#include "objects/collision_filters.h"

#include "objects/PlayerInst.h"

static lsound::Sound attack_sound;
void play(lsound::Sound& sound, const char* path);

static void get_visible_monsters(GameState* gs,
        std::vector<GameInst*>& visible_monsters, PlayerInst* p = NULL) {
    const std::vector<obj_id>& mids = gs->monster_controller().monster_ids();
    for (int i = 0; i < mids.size(); i++) {
        GameInst* inst = gs->get_instance(mids[i]);
        if (inst && gs->object_visible_test(inst, p)) {
            visible_monsters.push_back(inst);
        }
    }
}

static bool enemy_filter(GameInst* g1, GameInst* g2) {
    auto* c1 = dynamic_cast<CombatGameInst*>(g1);
    if (!c1) return false;
    auto* c2 = dynamic_cast<CombatGameInst*>(g2);
    if (!c2) return false;
    return c1->team != c2->team;
}


static GameInst* find_closest_from_list(GameState* gs,
        std::vector<GameInst*>& candidates, const Pos& pos,
        float* dist = NULL) {
    GameInst* closest = NULL;
    float mindist = -1;

    for (int i = 0; i < candidates.size(); i++) {
        GameInst* inst = candidates[i];
        float dx = inst->x - pos.x, dy = inst->y - pos.y;
        float dist = sqrt(dx * dx + dy * dy) - inst->target_radius;
        if (closest == NULL || dist < mindist) {
            mindist = dist;
            closest = inst;
        }
    }

    if (dist) {
        *dist = mindist;
    }

    return closest;
}

static GameInst* get_weapon_autotarget(GameState* gs, PlayerInst* p,
        GameInst* targ, float dx, float dy) {
    WeaponEntry& wentry = p->weapon().weapon_entry();
    Pos ppos(p->x, p->y);
    GameInst* inst = NULL;
    bool ismelee = !(wentry.uses_projectile || p->equipment().has_projectile());
    int target_range = wentry.range() + p->target_radius;

    if (targ) {
        int dist = distance_between(Pos(targ->x, targ->y), ppos)
                - targ->target_radius;
        if (dist <= target_range) {
            return targ;
        }
    }

    if (ismelee) {
        /*normalize dx & dy*/
        float mag = sqrt(dx * dx + dy * dy);
        if (mag == 0.0f)
            mag = 1.0f;
        dx *= target_range / mag, dy *= target_range / mag;

        /*test in our walking direction first*/
        gs->object_radius_test(p, &inst, 1, enemy_filter, p->x + dx,
                p->y + dy, p->radius);
    } else {
        target_range *= 1.25;
    }

    if (inst) {
        return inst;
    } else {
        GameInst* inst = get_nearest_visible_enemy(gs, p);
        if (inst && (!ismelee || distance_between(p->ipos(), inst->ipos()) <= target_range)) {
            return inst;
        }
    }
    return NULL;
}

bool find_safest_square(PlayerInst* p, GameState* gs, Pos& position) {
    PlayerData& pc = gs->player_data();

    std::vector<PlayerInst*> players = gs->players_in_level();
    std::vector<GameInst*> visible_monsters;
    // TODO

    int maxdist = 0;
    for (int i = 0; i < players.size(); i++) {
        BBox fbox = players[i]->field_of_view->tiles_covered();
        FOR_EACH_BBOX(fbox, x, y) {
            if (players[i]->field_of_view->within_fov(x, y)) {
                Pos pos(x * TILE_SIZE + TILE_SIZE / 2,
                        y * TILE_SIZE + TILE_SIZE / 2);

                if (gs->solid_test(p, pos.x, pos.y))
                    continue;

                float mindist;
                find_closest_from_list(gs, visible_monsters, pos, &mindist);

                if (mindist > TILE_SIZE / 2 && mindist > maxdist) {
                    maxdist = mindist;
                    position = pos;
                }
            }
        }
    }
    if (maxdist != 0) {
        float dx = p->x - position.x, dy = p->y - position.y;
        float dist = sqrt(dx * dx + dy * dy);
        return (dist > TILE_SIZE / 2);
    } else {
        return false;
    }
}

static int get_targets(GameState* gs, PlayerInst* p, PosF xy, int rad,
        GameInst** enemies, int max_targets) {
    int numhit = gs->object_radius_test(p, enemies, max_targets,
            enemy_filter, xy.x, xy.y, rad);
    if (numhit < max_targets) {
        GameInst* inst = get_nearest_visible_enemy(gs, p);
        if (inst) {
            numhit += gs->object_radius_test(p, enemies + numhit,
                    max_targets - numhit, enemy_filter, inst->x, inst->y,
                    rad);
        }
    }
    return numhit;
}

static bool lua_spell_get_target(GameState* gs, PlayerInst* p, LuaValue& action,
        Pos& pos) {
    bool nilresult = false;
    lua_State* L = gs->luastate();
    obj_id target_id = p->target();
    GameInst* target = gs->get_instance(target_id);

    int beforecall_idx = lua_gettop(L);
    action.push();
    luawrap::push(L, p);
    if (!target) {
        lua_pushnil(L);
    } else {
        luawrap::push(L, target);
    }

    // Allow for multiple return values
    // read the stack size difference to find out how many were returned
    lua_call(L, 2, LUA_MULTRET);
    int nret = lua_gettop(L) - beforecall_idx;

    if (nret != 2 || lua_isnil(L, -1)) {
        nilresult = true;
    } else if (nret == 2) {
        pos.x = lua_tointeger(L, -2);
        pos.y = lua_tointeger(L, -1);
    }

    lua_pop(L, nret);

    return !nilresult;
}

bool PlayerInst::enqueue_not_enough_mana_actions(GameState* gs) {
    const int AUTOUSE_MANA_POTION_CNT = 2;
    int item_slot = inventory().find_slot(get_item_by_name("Mana Potion"));
    if (gs->game_settings().autouse_mana_potions
            && autouse_mana_potion_try_count >= AUTOUSE_MANA_POTION_CNT
            && item_slot != -1) {
        queued_actions.push_back(
                game_action(gs, this, GameAction::USE_ITEM, item_slot));
        autouse_mana_potion_try_count = 0;
        return true;
    } else {
        autouse_mana_potion_try_count++;
        return false;
    }
}
bool PlayerInst::enqueue_io_spell_actions(GameState* gs, bool* fallback_to_melee) {
    lua_State* L = gs->luastate();

    GameView& view = gs->view();
    IOController& io = gs->io_controller();
    SpellsKnown& spells = spells_known();

    bool perform_spell = false;

    int spell_to_cast = spell_selected();
    //Spell choice
    for (int i = 0; i < spells.amount(); i++) {
        if (io_value.use_spell_slot() == i) {
            perform_spell = true;
            spell_to_cast = i;
            break;
        }
    }

    if (spell_to_cast != spell_selected()) {
        queued_actions.push_back(
                game_action(gs, this, GameAction::CHOSE_SPELL, spell_to_cast));
    }

    bool auto_target = false;
    // We auto-target if a mouse is not used
    if (io_value.target_position() == PosF(0,0)) {
        auto_target = true;
    }
    *fallback_to_melee = false;

    if (spell_to_cast > -1 && perform_spell) {
        SpellEntry& spl_entry = spells.get_entry(spell_to_cast);

        bool triggered_already = (previous_spell_cast == spell_to_cast);
        previous_spell_cast = spell_to_cast;
        Pos target;
        bool can_trigger = !triggered_already
                || spl_entry.can_cast_with_held_key;
        bool can_target;
        if (auto_target) {
            can_target = lua_spell_get_target(gs, this,
                    spl_entry.autotarget_func, target);
        } else {
            // TODO have target_position here
            int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();
            target = Pos(rmx, rmy);
            can_target = true;
        }

        bool not_enough_mana = (spl_entry.mp_cost > core_stats().mp);
        bool cooldown_active = (cooldowns().spell_cooldowns[spl_entry.id] > 0);
        if (not_enough_mana || cooldown_active) {
            if (!triggered_already && can_target) {
                if (!not_enough_mana || !enqueue_not_enough_mana_actions(gs)) {
                    *fallback_to_melee = spl_entry.fallback_to_melee;
                }
            } else {
                *fallback_to_melee = spl_entry.fallback_to_melee;
            }
            return false;
        } else {
            autouse_mana_potion_try_count = 0;
        }

        if (can_trigger && can_target) {
            bool is_channeled = lcall_def(/*default*/ false, spl_entry.is_channeled_func,
                /*caster*/ this, target.x, target.y);
            bool can_use = lcall_def(/*default*/ true, spl_entry.prereq_func,
                    /*caster*/ this, target.x, target.y);
            if (can_use) {
                queued_actions.push_back(
                    game_action(gs, this,
                                GameAction::USE_SPELL,
                                spell_to_cast, target.x, target.y));
                return true;
            } else if (is_channeled) {
                queued_actions.push_back(
                    game_action(gs, this,
                                GameAction::CHANNEL_SPELL,
                                spell_to_cast, target.x, target.y));
                return true;
            } else if (!auto_target) {
                gs->game_chat().add_message("Target location is not valid.");
            } else {
                *fallback_to_melee = spl_entry.fallback_to_melee;
            }
        } else if (!triggered_already && !can_target) {
            gs->game_chat().add_message(
                    "Cannot currently auto-target spell. Use manual controls (with mouse).");
            *fallback_to_melee = spl_entry.fallback_to_melee;
        }
    } else {
        previous_spell_cast = -1;
    }
    return false;
}

static bool decide_attack_movement(const Pos& player, const Pos& target,
        float threshold, int& vx, int& vy) {
    float dx = target.x - player.x, dy = target.y - player.y;
    if (fabs(dx) < threshold) {
        dx = 0;
    }
    if (fabs(dy) < threshold) {
        dy = 0;
    }
    normalize(dx, dy);
    vx = 0, vy = 0;
    if (dx > 0.2) {
        vx = 1;
    } else if (dx < -0.2) {
        vx = -1;
    }
    if (dy > 0.2) {
        vy = 1;
    } else if (dy < -0.2) {
        vy = -1;
    }
    return vx != 0 || vy != 0;
}

// dx & dy indicates moving direction, useful for choosing melee attack targets
bool PlayerInst::enqueue_io_spell_and_attack_actions(GameState* gs, float dx,
        float dy) {
    GameView& view = gs->view();
    WeaponEntry& wentry = weapon().weapon_entry();

    bool mouse_within = gs->mouse_x() < gs->view().width;
    int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

    int level = gs->get_level()->id(), frame = gs->frame();

    bool is_moving = (dx != 0.0f || dy != 0.0f);
    IOController& io = gs->io_controller();

    bool fallback_to_melee = false;

    bool attack_used = enqueue_io_spell_actions(gs, &fallback_to_melee);

    bool autotarget = (io_value.target_position() == PosF());

    bool weaponuse = false;

    // choose & use weapon
    if (io_value.should_use_weapon()) {
        queued_actions.push_back(
                game_action(gs, this, GameAction::CHOSE_SPELL, -1));
        weaponuse = true;
    }

    if (fallback_to_melee) {
        weaponuse = true;
    }

    // weapon use
    if (!attack_used && weaponuse) {

        bool is_projectile = wentry.uses_projectile
                || equipment().has_projectile();

        MonsterController& mc = gs->monster_controller();
        GameInst* curr_target = gs->get_instance(current_target);
        GameInst* target = NULL;
        Pos targ_pos;

        if (is_projectile) {
            if (!autotarget) {
                targ_pos = Pos(rmx, rmy);
            } else if (autotarget && curr_target) {
                targ_pos = curr_target->ipos();
            }
        } else {
            if (autotarget) {
                dx = rmx - x, dy = rmy - y;
            }
            target = get_weapon_autotarget(gs, this, curr_target, dx, dy);
            if (target) {
                targ_pos = Pos(target->x, target->y);

            }
            if (!is_moving && !target && autotarget && spell_selected() == -1
                    && curr_target && !is_projectile) {
                int vx, vy;
                GameInst* closest = get_nearest_visible_enemy(gs, this);

                    if (closest
                        && decide_attack_movement(ipos(), closest->ipos(),
                                TILE_SIZE / 4, vx, vy)) {
                    queued_actions.push_back(
                            game_action(gs, this, GameAction::MOVE, spellselect,
                                    round(vx), round(vy)));
                }
            }
        }
        if (target || (is_projectile && (!autotarget || curr_target))) {
            queued_actions.push_back(
                    game_action(gs, this, GameAction::USE_WEAPON, spellselect,
                            targ_pos.x, targ_pos.y));
            attack_used = true;
        }
    }
    return attack_used;
}

// Happens when you use up a projectile
// -- next try option some other weapon projectile
// -- next try unarmed projectile (redundant if already prefer unarmed)
// -- next try melee weapons
// -- next try unarmed melee
static void exhaust_projectile_autoequip(PlayerInst* player,
        const std::string& preferred_class) {
    if (projectile_smart_equip(player->inventory(), preferred_class)) {
        return;
    }
    weapon_smart_equip(player->inventory());
}

void PlayerInst::_use_weapon(GameState *gs, const GameAction &action) {
    // Dead players can't attack:
    if (is_ghost()) {
        return;
    }
    if (!effective_stats().allowed_actions.can_use_weapons) {
        return;
    }

    WeaponEntry& wentry = weapon().weapon_entry();
    MTwist& mt = gs->rng();
    const int MAX_MELEE_HITS = 10;
    EffectiveStats& estats = effective_stats();
    if (!cooldowns().can_doaction()) {
        return;
    }

    PosF start = pos();
    PosF actpos {action.action_x, action.action_y};

    if (wentry.uses_projectile && !equipment().has_projectile()) {
        return;
    }

    int cooldown = 0;

    if (equipment().has_projectile()) {
        Projectile projectile = equipment().projectile();
        ProjectileEntry& pentry = projectile.projectile_entry();

        int weaprange = pentry.range();
        float movespeed = pentry.speed;
        cooldown = pentry.cooldown();
        AttackStats weaponattack {Weapon(), projectile};

        if (wentry.weapon_class == pentry.weapon_class && !pentry.is_standalone()) {
            // Take into account weapon (only) if it matches the projectile class and projectile is not standalone
            weaprange = wentry.range();
            cooldown = wentry.cooldown();
            weaponattack = {weapon(), projectile};
        }

        gs->add_instance<ProjectileInst>(
            projectile, effective_atk_stats(mt, weaponattack),
            id, start.to_pos(), actpos.to_pos(),
            movespeed, weaprange, NONE, pentry.can_wall_bounce, pentry.can_wall_bounce ? 3 : 0);
        if (!equipment().use_ammo()) {
            exhaust_projectile_autoequip(this, this->last_chosen_weaponclass);
        }
        cooldowns().reset_action_cooldown(
            cooldown * estats.cooldown_modifiers.ranged_cooldown_multiplier);
    } else if (!wentry.attack.alt_action.isnil()) {
        auto* target_object = gs->get_instance(target());
        lcall(wentry.attack.alt_action, this, actpos, target_object);
        // TODO not necessarily:
        cooldowns().reset_action_cooldown(
            cooldown * estats.cooldown_modifiers.spell_cooldown_multiplier);
    } else {
        float weap_range = wentry.range() + this->radius + TILE_SIZE / 2;
        float distance = distance_between(actpos, pos());
        if (distance > weap_range) {
            LANARTS_ASSERT(distance > 0);
            // Find the unit direction vector towards the target
            PosF dir = {
                (actpos.x - x) / distance,
                (actpos.y - y) / distance
            };
            // Find the attack point in the direction of attack
            actpos = pos() + dir.scaled(weap_range);
        }

        GameInst* enemies[MAX_MELEE_HITS];

        const int max_targets = 1;
        const int dmgradius = 4;

        int numhit = get_targets(
            gs, this,
            actpos, dmgradius, enemies, max_targets
        );

        // TODO weapon attack hook
        for (int i = 0; i < numhit; i++) {
            auto* e = (EnemyInst*)enemies[i];
            attack(gs, e, AttackStats(equipment().weapon()));
        }
        if (numhit > 0) {
            cooldown = wentry.cooldown();
        }
        cooldowns().reset_action_cooldown(
            cooldown * estats.cooldown_modifiers.melee_cooldown_multiplier);
    }

    reset_rest_cooldown();
}

bool PlayerInst::melee_attack(GameState* gs, CombatGameInst* e,
        const Item& weapon, bool ignore_cooldowns,
        float damage_multiplier) {
    gs->for_screens([&]() {
    if (gs->local_player()->current_floor == current_floor) {
        play(attack_sound, "sound/melee.ogg");
    }});
    // Killed ? 
    return CombatGameInst::melee_attack(gs, e, weapon, ignore_cooldowns);
}

void PlayerInst::_use_spell(GameState *gs, const GameAction &action) {
    // Dead players can't use spells:
    if (is_ghost()) {
        return;
    }
    if (spells_known().amount() <= action.use_id) {
        // This spell is no longer valid since being queued!
        return;
    }
    spell_id spell = spells_known().get(action.use_id);
    SpellEntry& spl_entry = res::spell(spell);
    try_use_spell(
        gs, spl_entry,
        {action.action_x, action.action_y},
        gs->get_instance(target())
    );
}

void PlayerInst::_channel_spell(GameState *gs, const GameAction &action) {
    // Dead players can't channel spells:
    if (is_ghost()) {
        return;
    }
    if (spells_known().amount() <= action.use_id) {
        // This spell is no longer valid since being queued!
        return;
    }

    spell_id spell = spells_known().get(action.use_id);
    SpellEntry& spl_entry = res::spell(spell);

    bool is_channeled = lcall_def(/*default*/ false, spl_entry.is_channeled_func,
        /*caster*/ this, action.action_x, action.action_.y);
    if (!is_channeled) {
        // This spell is no longer valid since being queued!
        return;
    }

    lcall(spl_entry.on_channel_func, this,
        {action.action_x, action.action_y},
        gs->get_instance(target())
    );
}
