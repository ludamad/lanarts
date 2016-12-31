/*
 * PlayerInstAttackActions.cpp
 *  Implements the various attack and misc spell actions the player can perform.
 */

#include <lua.hpp>

#include <luawrap/luawrap.h>

#include "data/game_data.h"
#include "draw/colour_constants.h"
#include "draw/draw_sprite.h"
#include "draw/SpriteEntry.h"
#include "draw/TileEntry.h"
#include "gamestate/GameState.h"

#include "lua_api/lua_api.h"
#include "lua_api/lua_api.h"

#include "stats/items/ItemEntry.h"

#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"

#include "stats/SpellEntry.h"

#include <lcommon/math_util.h>

#include "lanarts_defines.h"

#include "../enemy/EnemyInst.h"

#include "../AnimatedInst.h"
#include "../ItemInst.h"
#include "../ProjectileInst.h"
#include "../collision_filters.h"

#include "PlayerInst.h"

static lsound::Sound attack_sound;
void play(lsound::Sound& sound, const char* path);

// equip.ogg  item.ogg  LICENSE_death_hiss    LICENSE_death_slime   LICENSE_death_yelp  minor_missile.ogg  player_hurt.ogg  slash.ogg
// gold.ogg   LICENSE   LICENSE_death_player  LICENSE_death_squeak  melee.ogg           overworld.ogg      see_monster.ogg
//
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

static GameInst* get_closest_monster(GameState* gs, PlayerInst* p) {
    std::vector<GameInst*> visible_monsters;
    get_visible_monsters(gs, visible_monsters, p);

    return find_closest_from_list(gs, visible_monsters, p->ipos());
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
        gs->object_radius_test(p, &inst, 1, enemy_colfilter, p->x + dx,
                p->y + dy, p->radius);
    } else {
        target_range *= 1.25;
    }

    if (inst) {
        return inst;
    } else {
        std::vector<GameInst*> visible_monsters;
        get_visible_monsters(gs, visible_monsters, p);

        float dist;
        GameInst* inst = find_closest_from_list(gs, visible_monsters, ppos,
                &dist);
        if (inst && (!ismelee || dist <= target_range)) {
            return inst;
        }
    }
    return NULL;
}

bool find_safest_square(PlayerInst* p, GameState* gs, Pos& position) {
    PlayerData& pc = gs->player_data();

    std::vector<PlayerInst*> players = gs->players_in_level();
    std::vector<GameInst*> visible_monsters;
    get_visible_monsters(gs, visible_monsters);

    int maxdist = 0;
    for (int i = 0; i < players.size(); i++) {
        BBox fbox = players[i]->field_of_view().tiles_covered();
        FOR_EACH_BBOX(fbox, x, y) {
            if (players[i]->field_of_view().within_fov(x, y)) {
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

static int get_targets(GameState* gs, PlayerInst* p, int ax, int ay, int rad,
        GameInst** enemies, int max_targets) {
    int numhit = gs->object_radius_test(p, enemies, max_targets,
            enemy_colfilter, ax, ay, rad);
    if (numhit < max_targets) {
        std::vector<GameInst*> visible_monsters;
        get_visible_monsters(gs, visible_monsters, p);

        GameInst* inst = find_closest_from_list(gs, visible_monsters,
                Pos(p->x, p->y));

        if (inst) {
            numhit += gs->object_radius_test(p, enemies + numhit,
                    max_targets - numhit, enemy_colfilter, inst->x, inst->y,
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

static void player_use_luacallback_spell(GameState* gs, PlayerInst* p,
        SpellEntry& spl_entry, LuaValue& action, const Pos& target) {
    lua_State* L = gs->luastate();
    action.push();
    luawrap::push(L, p);
    lua_pushnumber(L, target.x);
    lua_pushnumber(L, target.y);
    luawrap::push(L, gs->get_instance(p->target()));
    lua_call(L, 4, 0);
}

static bool lua_spell_check_prereq(GameState* gs, PlayerInst* p,
        SpellEntry& spl_entry, LuaValue& action, const Pos& target) {
    lua_State* L = gs->luastate();
    bool passes = true;

    if (!action.empty()) {
        action.push();
        luawrap::push(L, p);
        lua_pushnumber(L, target.x);
        lua_pushnumber(L, target.y);
        lua_call(L, 3, 1);
        passes = lua_toboolean(L, -1);
        lua_pop(L, 1);
    }

    return passes;
}

const float PI = 3.141592f;

static void player_use_projectile_spell(GameState* gs, PlayerInst* p,
        SpellEntry& spl_entry, const Projectile& projectile,
        const Pos& target) {
    MTwist& mt = gs->rng();
    AttackStats projectile_attack(Weapon(), projectile);
    ProjectileEntry& pentry = projectile.projectile_entry();
    bool wallbounce = pentry.can_wall_bounce, passthrough = pentry.can_pass_through;
    int nbounces = pentry.number_of_target_bounces;
    float speed = pentry.speed * p->effective_stats().core.spell_velocity_multiplier;

    if (spl_entry.name == "Mephitize" || spl_entry.name == "Trepidize") {
        float vx = 0, vy = 0;
        direction_towards(Pos {p->x, p->y}, target, vx, vy, 10000);
        int directions = (spl_entry.name == "Trepidize" ? 4 : 16);

        for (int i = 0; i < directions; i++) {
            float angle = PI / directions * 2 * i;
            Pos new_target {p->x + cos(angle) * vx - sin(angle) * vy, p->y + cos(angle) * vy + sin(angle) * vx};
            GameInst* pinst = new ProjectileInst(projectile,
                    p->effective_atk_stats(mt, projectile_attack), p->id,
                    Pos(p->x, p->y), new_target, speed, pentry.range(), NONE,
                    wallbounce, nbounces, passthrough);
            gs->add_instance(pinst);
        }

    } else {
        GameInst* pinst = new ProjectileInst(projectile,
                p->effective_atk_stats(mt, projectile_attack), p->id,
                Pos(p->x, p->y), target, speed, pentry.range(), NONE,
                wallbounce, nbounces, passthrough);
        gs->add_instance(pinst);
    }
}

static void player_use_spell(GameState* gs, PlayerInst* p,
        SpellEntry& spl_entry, const Pos& target) {
    lua_State* L = gs->luastate();

    p->core_stats().mp -= spl_entry.mp_cost;
    float spell_cooldown_mult =
            p->effective_stats().cooldown_modifiers.spell_cooldown_multiplier;
    p->cooldowns().reset_action_cooldown(
            spl_entry.cooldown * spell_cooldown_mult);
    // Set global cooldown for spell:
    p->cooldowns().spell_cooldowns[spl_entry.id] = std::max(int(spl_entry.spell_cooldown * spell_cooldown_mult), p->cooldowns().spell_cooldowns[spl_entry.id]);
    if (spl_entry.uses_projectile()) {
        player_use_projectile_spell(gs, p, spl_entry, spl_entry.projectile,
                target);
    } else {
        player_use_luacallback_spell(gs, p, spl_entry,
                spl_entry.action_func.get(L), target);
    }
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
    bool chose_spell = false;
    bool triggered_already = false;

    int newspell = spell_selected();
    //Spell choice
    for (int i = 0; i < spells.amount(); i++) {
        IOEvent event(IOEvent::ACTIVATE_SPELL_N, i);
        if (io.query_event(event, &triggered_already)) {
            chose_spell = true;
            // Use the remembered choice to determine if its appropriate to cast this spell
            // This makes sure the key must be hit once to switch spell, and again to use it
//            if (spell_selected() == i ) {
                // NB: Commenting out double hit for now, remove other comments when this is deemed superior
//                    && previous_spellselect == spell_selected()) {
                //Double hit a spell switch to quick-perform it
                perform_spell = true;
//            } else if (!triggered_already) {
                newspell = i;
//            }
            break;
        }
    }
    if (!chose_spell) {
        // If we did not switch a spell with one of the quick-select keys, remember our choice
        previous_spellselect = spell_selected();

        if (io.query_event(IOEvent::TOGGLE_ACTION_UP)) {
            if (spells.amount() > 0) {
                newspell = (spell_selected() + 1) % spells.amount();
            }
        } else if (io.query_event(IOEvent::TOGGLE_ACTION_DOWN)) {
            if (spell_selected() <= 0) {
                newspell = spells.amount() - 1;
            } else {
                newspell = spell_selected() - 1;
            }
        }
    }

    if (newspell != spell_selected()) {
        queued_actions.push_back(
                game_action(gs, this, GameAction::CHOSE_SPELL, newspell));
    }

    bool auto_target = true;
    // We don't auto-target unless a mouse is not used
    if (!perform_spell
            && io.query_event(IOEvent::MOUSETARGET_CURRENT_ACTION,
                    &triggered_already)) {
        perform_spell = true;
        auto_target = false;

    } else if (!perform_spell) {
        perform_spell = io.query_event(IOEvent::AUTOTARGET_CURRENT_ACTION,
                &triggered_already);
    }

    *fallback_to_melee = false;

    if (newspell > -1 && perform_spell) {
        SpellEntry& spl_entry = spells.get_entry(newspell);

        Pos target;
        bool can_trigger = !triggered_already
                || spl_entry.can_cast_with_held_key;
        bool can_target;
        if (auto_target) {
            can_target = lua_spell_get_target(gs, this,
                    spl_entry.autotarget_func.get(L), target);
        } else {
            int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();
            target = Pos(rmx, rmy);
            can_target = true;
        }

        if (spl_entry.mp_cost > core_stats().mp) {
            if (!triggered_already && can_target) {
                if (!enqueue_not_enough_mana_actions(gs)) {
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
            bool can_use = lua_spell_check_prereq(gs, this, spl_entry,
                    spl_entry.prereq_func.get(L), target);
            if (can_use) {
                queued_actions.push_back(
                        game_action(gs, this, GameAction::USE_SPELL,
                                newspell, target.x, target.y));
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

    bool autotarget = io.query_event(IOEvent::AUTOTARGET_CURRENT_ACTION)
            || io.query_event(IOEvent::ACTIVATE_SPELL_N);
    bool mousetarget = io.query_event(IOEvent::MOUSETARGET_CURRENT_ACTION);

    bool weaponuse = spell_selected() == -1;

    // choose & use weapon
    if (io.query_event(IOEvent::USE_WEAPON)) {
        queued_actions.push_back(
                game_action(gs, this, GameAction::CHOSE_SPELL, -1));
        autotarget = true;
        weaponuse = true;
    }

    if (fallback_to_melee) {
        weaponuse = true;
    }

    // weapon use
    if (!attack_used && weaponuse && (autotarget || mousetarget)) {

        bool is_projectile = wentry.uses_projectile
                || equipment().has_projectile();

        MonsterController& mc = gs->monster_controller();
        GameInst* curr_target = gs->get_instance(current_target);
        GameInst* target = NULL;
        Pos targ_pos;

        if (is_projectile) {
            if (mousetarget) {
                targ_pos = Pos(rmx, rmy);
            } else if (autotarget && curr_target) {
                targ_pos = curr_target->ipos();
            }
        } else {
            if (mousetarget) {
                dx = rmx - x, dy = rmy - y;
            }
            target = get_weapon_autotarget(gs, this, curr_target, dx, dy);
            if (target) {
                targ_pos = Pos(target->x, target->y);

            }
            if (!is_moving && !target && !mousetarget && spell_selected() == -1
                    && curr_target && !is_projectile) {
                int vx, vy;
                GameInst* closest = get_closest_monster(gs, this);

                if (closest
                        && decide_attack_movement(ipos(), closest->ipos(),
                                TILE_SIZE / 4, vx, vy)) {
                    queued_actions.push_back(
                            game_action(gs, this, GameAction::MOVE, spellselect,
                                    round(vx), round(vy)));
                }
            }
        }
        if (target || (is_projectile && (mousetarget || curr_target))) {
            queued_actions.push_back(
                    game_action(gs, this, GameAction::USE_WEAPON, spellselect,
                            targ_pos.x, targ_pos.y));
            attack_used = true;
        }
    }
    return attack_used;
}

static void lua_hit_callback(lua_State* L, LuaValue& callback, GameInst* user,
        GameInst* target) {
    if (!callback.empty()) {
        callback.push();
        luawrap::push(L, user);
        luawrap::push(L, target);
        lua_call(L, 2, 0);
    }
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

void PlayerInst::use_weapon(GameState* gs, const GameAction& action) {
    if (!effective_stats().allowed_actions.can_use_weapon) {
        return;
    }
    lua_State* L = gs->luastate();

    WeaponEntry& wentry = weapon().weapon_entry();
    MTwist& mt = gs->rng();
    const int MAX_MELEE_HITS = 10;
    EffectiveStats& estats = effective_stats();
    if (!cooldowns().can_doaction()) {
        return;
    }

    Pos start(x, y);
    Pos actpos(action.action_x, action.action_y);

    if (wentry.uses_projectile && !equipment().has_projectile()) {
        return;
    }

    int cooldown = 0;

    if (equipment().has_projectile()) {
        Projectile projectile = equipment().projectile();
        ProjectileEntry& pentry = projectile.projectile_entry();
        item_id item = get_item_by_name(pentry.name.c_str());

        int weaprange = pentry.range();
        float movespeed = pentry.speed;
        cooldown = pentry.cooldown();
        AttackStats weaponattack {Weapon()};

        if (wentry.weapon_class == pentry.weapon_class) {
            // Take into account weapon (only) if it matches the projectile class
            weaprange = wentry.range();
            cooldown = wentry.cooldown();
            weaponattack = {weapon()};
        }

        bool wallbounce = false;
        int nbounces = 0;

        if (class_stats().class_entry().name != "Ranger" && wentry.weapon_class == "bows") {
//            weaprange = TILE_SIZE * 2.3;
            cooldown *= 1.5;
        }
        GameInst* bullet = new ProjectileInst(projectile,
                effective_atk_stats(mt, weaponattack), id, start, actpos,
                movespeed, weaprange, NONE, wallbounce, nbounces);
        gs->add_instance(bullet);
        if (!equipment().use_ammo()) {
            exhaust_projectile_autoequip(this, this->last_chosen_weaponclass);
        }
    } else {
        int weaprange = wentry.range() + this->radius + TILE_SIZE / 2;
        float mag = distance_between(actpos, Pos(x, y));
        if (mag > weaprange) {
            float dx = actpos.x - x, dy = actpos.y - y;
            actpos = Pos(x + dx / mag * weaprange, y + dy / mag * weaprange);
        }

        GameInst* enemies[MAX_MELEE_HITS];

        const int max_targets = 1;
        const int dmgradius = 4;

        int numhit = get_targets(gs, this, actpos.x, actpos.y, dmgradius,
                enemies, max_targets);

        if (numhit == 0) {
            return;
        }

        for (int i = 0; i < numhit; i++) {
            EnemyInst* e = (EnemyInst*)enemies[i];
            lua_hit_callback(gs->luastate(), wentry.action_func().get(L), this,
                    e);
            attack(gs, e, AttackStats(equipment().weapon()) );
        }
        cooldown = wentry.cooldown();
    }

    float cooldown_mult;
    if (equipment().has_projectile()) {
        cooldown_mult = estats.cooldown_modifiers.ranged_cooldown_multiplier;
    } else {
        cooldown_mult = estats.cooldown_modifiers.melee_cooldown_multiplier;
    }
    cooldowns().reset_action_cooldown(cooldown * cooldown_mult);

    reset_rest_cooldown();
}

bool PlayerInst::melee_attack(GameState* gs, CombatGameInst* e,
        const Item& weapon, bool ignore_cooldowns) {
    if (gs->local_player()->current_floor == current_floor) {
        play(attack_sound, "sound/melee.ogg");
    }
    // Killed ? 
    if (CombatGameInst::melee_attack(gs, e, weapon, ignore_cooldowns) && dynamic_cast<EnemyInst*>(e)) {
        PlayerData& pc = gs->player_data();
        signal_killed_enemy();

        char buffstr[32];
        double xpworth = ((EnemyInst*)e)->xpworth();
        double n_killed = (pc.n_enemy_killed(((EnemyInst*) e)->enemy_type()) - 1) / pc.all_players().size();
        xpworth *= pow(0.9, n_killed);
        if (n_killed > 15) {
            xpworth = 0;
        }
        int amnt = round(xpworth / pc.all_players().size());

        players_gain_xp(gs, amnt);
        snprintf(buffstr, 32, "%d XP", amnt);
        gs->add_instance(
                new AnimatedInst(Pos(e->x - 5, e->y - 5), -1, 25,
                        PosF(), PosF(), AnimatedInst::DEPTH, buffstr,
                        Colour(255, 215, 11)));
        return true;
    }
    return false;
}

void PlayerInst::use_spell(GameState* gs, const GameAction& action) {
    if (!effective_stats().allowed_actions.can_use_spells) {
        return;
    }
    MTwist& mt = gs->rng();
    EffectiveStats& estats = effective_stats();

    spell_id spell = spells_known().get(action.use_id);
    SpellEntry& spl_entry = res::spell(spell);

    if (cooldowns().spell_cooldowns[spell] > 0) {
        return;
    }
    if (spl_entry.mp_cost > core_stats().mp
            || (!spl_entry.can_cast_with_cooldown && !cooldowns().can_doaction())) {
        return;
    }

    Pos target = Pos(action.action_x, action.action_y);
    player_use_spell(gs, this, spl_entry, target);

    cooldowns().action_cooldown *= estats.cooldown_mult;
    cooldowns().reset_rest_cooldown(REST_COOLDOWN);
}
