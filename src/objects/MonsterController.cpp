/*
 * MonsterController.cpp:
 *  Centralized location of all pathing decisions of monsters, with collision avoidance
 */

#include <algorithm>
#include <cmath>

#include <rvo2/RVO.h>

#include <ldraw/draw.h>


#include "draw/colour_constants.h"
#include "draw/TileEntry.h"
#include "gamestate/GameState.h"

#include "gamestate/PlayerData.h"
#include "gamestate/TeamIter.h"

#include "stats/items/WeaponEntry.h"
#include "stats/effect_data.h"
#include "stats/items/ProjectileEntry.h"

#include <lcommon/math_util.h>

#include "objects/PlayerInst.h"
#include "objects/CombatGameInstFunctions.h"
#include "objects/collision_filters.h"

#include "EnemyInst.h"
#include "MonsterController.h"

const int HUGE_DISTANCE = 1000000;

MonsterController::MonsterController() {
}

MonsterController::~MonsterController() {
}


static bool _can_head(Grid<bool>& solidity, const BBox& bbox, float dx, float dy) {
    bool is_diag = (fabs(dx) > 0 && fabs(dy) > 0);

    int xx, yy;
    for (int y = bbox.y1; y <= bbox.y2 + TILE_SIZE; y += TILE_SIZE) {
        for (int x = bbox.x1; x <= bbox.x2 + TILE_SIZE; x += TILE_SIZE) {
            xx = squish(x, bbox.x1, bbox.x2 + 1);
            yy = squish(y, bbox.y1, bbox.y2 + 1);

            int gx = (xx + dx) / TILE_SIZE;
            int gy = (yy + dy) / TILE_SIZE;
            if (solidity[{gx, gy}]) {
                return false;
            }
            if (is_diag) {
                if (solidity[{xx / TILE_SIZE, gy}]) {
                    return false;
                }
                if (solidity[{gx, yy / TILE_SIZE}]) {
                    return false;
                }
            }
        }
    }
    return true;
}

static void _least_smell_near(Grid<bool>& solidity, Grid<float>& smells, const Pos xy, float& iter_least, Pos& iter_loc) {
    BBox area {xy - Pos{1,1}, Size{3    ,3}};
    FOR_EACH_BBOX(area, x, y) {
        if ((xy.x == x && xy.y == y) || solidity[{x,y}]) {
            continue;
        }
        Pos d = Pos {x, y} - xy;
        if (abs(d.x) == 1 && abs(d.y) == 1) {
            // Check if diagonally blocked:
            if (solidity[{x, xy.y}] && solidity[{xy.x, y}]) {
                continue;
            }
        }
        if (smells[{x, y}] < iter_least) {
            iter_least = smells[{x,y}];
            iter_loc = {x, y};
        }
    }
}

PosF configure_dir(GameState* gs, CombatGameInst* inst, float dx, float dy);
PosF lite_configure_dir(GameState* gs, CombatGameInst* inst, float dx, float dy);
static PosF _towards_least_smell(Grid<bool>& solidity, Grid<float>& smells, const BBox& bbox, float speed) {
    using namespace std;

    //Set up coordinate min and max
    int mingrid_x = bbox.x1 / TILE_SIZE, mingrid_y = bbox.y1 / TILE_SIZE;
    int maxgrid_x = bbox.x2 / TILE_SIZE, maxgrid_y = bbox.y2 / TILE_SIZE;
    //Make sure coordinates do not go out of bounds
    BBox area = {
        squish(mingrid_x, 2, smells.width() - 2),
        squish(mingrid_y, 2, smells.height() - 2),
        squish(maxgrid_x, 2, smells.width() - 2) + 1,
        squish(maxgrid_y, 2, smells.height() - 2) + 1};

    float acc_x = 0, acc_y = 0;
    FOR_EACH_BBOX(area, xx, yy) {
        int sx = max(xx * TILE_SIZE, bbox.x1), sy = max(yy * TILE_SIZE,
                                                        bbox.y1);
        int ex = min((xx + 1) * TILE_SIZE, bbox.x2), ey = min(
                (yy + 1) * TILE_SIZE, bbox.y2);

        float least = INFINITY;
        Pos least_xy = {xx, yy};
        if (!solidity[{xx,yy}]) {
            _least_smell_near(solidity, smells, {xx,yy}, least, least_xy);
            PosF dir = (least_xy) - Pos(xx,yy);
            int sub_area = (ex - sx) * (ey - sy) + 1;
            /*Make sure all interpolated directions are possible*/
            acc_x += dir.x * sub_area;
            acc_y += dir.y * sub_area;
        }
    }
    float mag = sqrt(float(acc_x * acc_x + acc_y * acc_y));
    if (mag == 0) {
        return PosF();
    } else {
        float vx = speed * float(acc_x) / mag;
        float vy = speed * float(acc_y) / mag;
        return PosF(vx, vy);
    }
}

PosF MonsterController::towards_least_smell(GameState *gs, CombatGameInst *inst) {
    if (smell_map.empty()) {
        return PosF{0, 0};
    }
    return _towards_least_smell(*inst->get_map(gs)->tiles().solidity_map(), smell_map, inst->bbox(), inst->effective_stats().movespeed);
}

void MonsterController::register_enemy(GameInst* enemy) {
    mids.push_back(enemy->id);
    RVO::Vector2 enemy_position(enemy->x, enemy->y);
    EnemyInst* e = (EnemyInst*)enemy;
    EffectiveStats& estats = e->effective_stats();
    EnemyBehaviour& eb = e->behaviour();
}

void MonsterController::serialize(SerializeBuffer& serializer) {
    serializer.write_container(mids);
    serializer.write(monsters_wandering_flag);
}

void MonsterController::deserialize(SerializeBuffer& serializer) {
    serializer.read_container(mids);
    serializer.read(monsters_wandering_flag);
}

CombatGameInst* MonsterController::find_actor_to_target(GameState* gs, EnemyInst* e) {
    //Use a 'GameView' object to make use of its helper methods
    GameView view(0, 0, PLAYER_PATHING_RADIUS * 2, PLAYER_PATHING_RADIUS * 2,
            gs->width(), gs->height());

    //Determine which players we are currently in view of
    BBox ebox = e->bbox();
    int mindistsqr = HUGE_DISTANCE;
    CombatGameInst* closest_actor = NULL;
    for_all_enemies(gs->team_data(), e, [&](CombatGameInst* actor) {
        bool isvisible = is_visible(gs, e, actor);
        if (isvisible) {
            PlayerInst* p = NULL;
            // HACK TODO
            if ( (p = dynamic_cast<PlayerInst*>(actor) )) {
                p->rest_cooldown() = REST_COOLDOWN;
            }
        }
        view.sharp_center_on(actor->x, actor->y);
        bool chasing = e->behaviour().chase_timeout > 0
                && actor->id == e->behaviour().chasing_actor;
        bool forced_wander = e->effects.has("Dazed");
        event_log("View %d %d\n", view.x, view.y);
        if (view.within_view(ebox) && (chasing || isvisible) && !forced_wander) {
            e->behaviour().current_action = EnemyBehaviour::CHASING_PLAYER;

            int dx = e->x - actor->x, dy = e->y - actor->y;
            int distsqr = dx * dx + dy * dy;
            event_log("Enemy id=%d name=%s considering target id=%d, dx=%d, dy=%d\n", e->id, e->etype().name.c_str(), actor->id, dx, dy);
            if (distsqr > 0 /*overflow check*/&& distsqr < mindistsqr) {
                mindistsqr = distsqr;
                closest_actor = actor;
            }
        }
    });
    return closest_actor;
}

bool potentially_randomize_movement(GameState* gs, EnemyInst* e);

bool attack_ai_choice(GameState* gs, CombatGameInst* inst,
                      CombatGameInst* target, AttackStats& attack);
bool go_towards_if_free_in_direction(GameState* gs, CombatGameInst* inst,
                                     float vx, float vy);
bool same_target_and_moved_colfilter(GameInst* self, GameInst* other);

void MonsterController::pre_step(GameState* gs) {
    perf_timer_begin(FUNCNAME);
    if (handler.empty()) {
        gs->game_world().push_level_object(gs->get_level_id());
        handler = lcall_def(LuaValue(), luawrap::globals(gs->luastate())["Engine"]["monster_controller"],
                            LuaStackValue(gs->luastate(), -1));
        lua_pop(gs->luastate(), 1);
    }

    auto base = LuaValue::newtable(gs->luastate());
    base["smell_map_step"] = [=]() {
        if (smell_map.empty()) {
            smell_map.resize(gs->tiles().size());
            swap_smell_map.resize(gs->tiles().size());
        } else {
            // Calculate new smell map:
            BBox area{{1, 1}, smell_map.size() - Size(2, 2)};
            Grid<bool> &solid = *gs->tiles().solidity_map();
            //        swap_smell_map.fill(0.0f);
            FOR_EACH_BBOX(area, x, y) {
                    if (solid[{x, y}]) {
                        continue;
                    }
                    BBox subarea = {{x - 1, y - 1}, Size(3, 3)};
                    float val = 0;
                    float n_neighbours = 0;
                    FOR_EACH_BBOX(subarea, xx, yy) {
                            if (solid[{xx, yy}]) {
                                continue;
                            }
                            val += smell_map[{xx, yy}];
                            n_neighbours += 1;
                        }
                    swap_smell_map[{x, y}] = (val / n_neighbours) * 0.2 + smell_map[{x, y}] * 0.79;
                }
            smell_map._internal_vector().swap(swap_smell_map._internal_vector());
        }
    };
    base["towards_least_smell"] = [=](CombatGameInst *obj) {
        return this->towards_least_smell(gs, obj);
    };
    base["ensure_legal_direction"] = [&](CombatGameInst *obj, PosF dir) {
        return lite_configure_dir(gs, obj, dir.x, dir.y);
    };
    base["add_object_smell"] = [=](CombatGameInst* obj, double value) {
        smell_map[obj->ipos().divided(TILE_SIZE)] += value;
    };
    base["attempt_move_object"] = [=](CombatGameInst* obj, PosF pos) {
        obj->attempt_move_to_position(gs, pos);
    };
    base["rvo_adjust_headings"] = [=]() {
        CollisionAvoidance& coll_avoid = gs->collision_avoidance();
        //Update player positions for collision avoidance simulator
        for (PlayerInst* p : players) {
            coll_avoid.set_position(p->collision_simulation_id(), p->x, p->y);
        }

        for (int id : mids) {
            EnemyInst* e = (EnemyInst*)gs->get_instance(id);
            if (!e) {
                continue;
            }
            update_velocity(gs, e);
            simul_id simid = e->collision_simulation_id();
            coll_avoid.set_position(simid, e->x, e->y);
        }
        coll_avoid.step();
        for (int i = 0; i < mids.size(); i++) {
            EnemyInst* e = (EnemyInst*)gs->get_instance(mids[i]);
            if (!e) {
                continue;
            }
            if (gs->tile_radius_test(e->x, e->y, TILE_SIZE / 2 + 4)) {
                continue;
            }
            CollisionAvoidance &coll_avoid = gs->collision_avoidance();
            simul_id simid = e->collision_simulation_id();
            PosF pos = coll_avoid.get_position(simid);
            e->vx = pos.x - e->pos().x;
            e->vy = pos.y - e->pos().y;
        }
    };

    base["try_attack"] = [=](EnemyInst* e) {
        CombatGameInst *p = get_nearest_enemy(gs, e);
        e->behaviour().movement_decided = false;
        if (p == NULL) {
            return;
        }
        e->behaviour().chasing_actor = p->id;
        // Compare position to enemy of interest:
        float pdist = distance_between(Pos(e->x, e->y), Pos(p->x, p->y));

        AttackStats attack;
        bool viable_attack = attack_ai_choice(gs, e, p, attack);
        WeaponEntry &wentry = attack.weapon_entry();
        bool hasproj = attack.projectile.id != NO_ITEM;

        // Part of: Implement a bunch of new status effects.
        bool has_fear = e->effects.has("Fear");
        if (pdist < e->target_radius + p->target_radius && !has_fear) {
            e->vx = 0, e->vy = 0;
        }

        if (viable_attack) {
            int mindist = wentry.range() + p->target_radius + e->target_radius
                          - TILE_SIZE / 8;
            if (hasproj) {
                mindist = attack.projectile_entry().range();
            }
            if (has_fear) {
                // Don't stop moving near if have fear.
            } else if (!attack.is_ranged()) {
                e->vx = 0, e->vy = 0;
            } else {
                int close = 40;
                bool stop_once_in_range = e->effects.has("StopOnceInRange");
                if (stop_once_in_range) {
                    close = 150;
                }
                if (pdist < std::min(mindist, close)) {
                    e->vx = 0, e->vy = 0;
                }
            }
            e->attack(gs, p, attack);

        }
    };
    base["adjust_heading_if_near_wall"] = [=](EnemyInst* e) {
        auto* p = get_nearest_enemy(gs, e);
        if (p && gs->tile_radius_test(e->x, e->y, TILE_SIZE / 2 + 4)) {
            if (gs->object_radius_test(e, NULL, 0,
                                       same_target_and_moved_colfilter, e->x, e->y,
                                       e->target_radius - e->effective_stats().movespeed - 2)) {
                float dx = p->x - e->x, dy = p->y - e->y;
                if (!go_towards_if_free_in_direction(gs, e, -dy, dx)
                    && !go_towards_if_free_in_direction(gs, e, dy, -dx)) {
                    e->vx = 0, e->vy = 0;
                }
            }
            e->behaviour().movement_decided = true;
        }
        event_log("set_monster_headings id=%d vx=%f vy=%f\n", e->id, e->vx, e->vy);
    };
    base["potentially_randomize_movement"] = [=](EnemyInst* e) {
        return potentially_randomize_movement(gs, e);
    };

//    coll_avoid.step();

//    for (int i = 0; i < mids.size(); i++) {
//        EnemyInst* e = (EnemyInst*)gs->get_instance(mids[i]);
//                if (!e) {
//                    continue;
//                }
//        update_position(gs, e);
//    }

    //Update 'mids' to only hold live objects
    std::vector<obj_id> mids2;
    mids2.reserve(mids.size());
    mids.swap(mids2);
    auto monsters = LuaValue::newtable(gs->luastate());
    for (int i = 0; i < mids2.size(); i++) {
        EnemyInst *e = (EnemyInst *) gs->get_instance(mids2[i]);
        if (e == NULL)
            continue;
        monsters[monsters.objlen() + 1] = e;
        EnemyBehaviour &eb = e->behaviour();
        eb.step();

        //Add live instances back to monster id list
        mids.push_back(mids2[i]);
    }

    players = gs->players_in_level();

    lmeth(handler, "step", base, monsters, players);
//
//    std::vector<EnemyOfInterest> eois;
//
//    for (int i = 0; i < mids.size(); i++) {
//        EnemyInst *e = (EnemyInst *) gs->get_instance(mids[i]);
//        CombatGameInst* actor = find_actor_to_target(gs, e);
//
//        // Part of: Implement status effects.
//        bool forced_wander = e->effects.has("Dazed");
//        if (forced_wander) {
//            if (eb.current_action == EnemyBehaviour::CHASING_PLAYER) {
//                eb.current_action = EnemyBehaviour::INACTIVE;
//            }
//        } else {
//            if (eb.current_action == EnemyBehaviour::INACTIVE
//                    && e->cooldowns().is_hurting()) {
//                eb.current_action = EnemyBehaviour::CHASING_PLAYER;
//            }
//            if (actor == NULL
//                    && eb.current_action == EnemyBehaviour::CHASING_PLAYER) {
//                eb.current_action = EnemyBehaviour::INACTIVE;
//                e->target() = NONE;
//            }
//        }
//
//
//        if (actor != NULL && eb.current_action == EnemyBehaviour::CHASING_PLAYER) {
//                event_log("Enemy id=%d has enemy of interest %d\n", e->id, actor->id);
//                eois.push_back(
//                    EnemyOfInterest(e, actor->id, inst_distance(e, actor))
//                );
//        } else {
//            e->vx = 0, e->vy = 0;
//            if (e->has_paths_data() && eb.current_action != EnemyBehaviour::FOLLOWING_PATH) {
//                GameInst* inst = get_nearest_ally(gs, e);
//                if (inst) {
//                    if (distance_between(e->ipos(), inst->ipos()) > TILE_SIZE * 3) {
//                        Pos p = e->direction_towards_ally_player(gs);
//                        e->vx = p.x, e->vy = p.y;
//                        float speed = e->effective_stats().movespeed;
//                        normalize(e->vx, e->vy, speed);
//                    }
//                }
//            }
//
//            if (e->vx == 0 && e->vy == 0) {
//                if (eb.current_action == EnemyBehaviour::INACTIVE) {
//                        monster_wandering(gs, e);
//                } else {
//                        //if (eb.current_action == EnemyBehaviour::FOLLOWING_PATH)
//                        monster_follow_path(gs, e);
//                }
//            }
//        }
//    }
//
//    set_monster_headings(gs, eois);
//
//    //Update player positions for collision avoidance simulator
//    for (int i = 0; i < players.size(); i++) {
//        PlayerInst* p = players[i];
//        coll_avoid.set_position(p->collision_simulation_id(), p->x, p->y);
//    }
//
//    for (int i = 0; i < mids.size(); i++) {
//        EnemyInst* e = (EnemyInst*)gs->get_instance(mids[i]);
//                if (!e) {
//                    continue;
//                }
//        lua_State* L = gs->luastate();
//        lua_gameinst_callback(L, e->etype().step_event.get(L), e);
//        update_velocity(gs, e);
//        simul_id simid = e->collision_simulation_id();
//        coll_avoid.set_position(simid, e->rx, e->ry);
//    }

//    coll_avoid.step();

//    for (int i = 0; i < mids.size(); i++) {
//        EnemyInst* e = (EnemyInst*)gs->get_instance(mids[i]);
//                if (!e) {
//                    continue;
//                }
//        update_position(gs, e);
//    }

    perf_timer_end(FUNCNAME);
}

void MonsterController::update_velocity(GameState* gs, EnemyInst* e) {
    float movespeed = e->effective_stats().movespeed;

    bool has_fear = e->effects.has("Fear");
    if (e->cooldowns().is_hurting()) {
        // Ogre mages in specific don't slow movement, and neither do fleeing enemies:
        if (e->etype().name == "Ogre Mage" || has_fear) {
        //// Hell forged's move faster, dissuading kiting:
        //} else if (e->etype().name == "Hell Forged") {
        //    e->vx *= 2, e->vy *= 2;
        //    movespeed *= 2;
        } else {
            e->vx /= 2, e->vy /= 2;
            movespeed /= 2;
        }
    }

    // Fear forces enemies to move backwards:
    if (has_fear) {
        e->vx *= -1, e->vy *= -1;
    }

    CollisionAvoidance& coll_avoid = gs->collision_avoidance();
    coll_avoid.set_preferred_velocity(e->collision_simulation_id(), e->vx,
            e->vy);
    coll_avoid.set_maxspeed(e->collision_simulation_id(), movespeed);
}
void MonsterController::update_position(GameState* gs, EnemyInst* e) {
    CollisionAvoidance& coll_avoid = gs->collision_avoidance();
    simul_id simid = e->collision_simulation_id();
    PosF pos = coll_avoid.get_position(simid);

    PosF new_xy = e->attempt_move_to_position(gs, pos);
    coll_avoid.set_position(simid, new_xy.x, new_xy.y);
    coll_avoid.set_maxspeed(simid, e->effective_stats().movespeed);
}

void MonsterController::post_draw(GameState* gs) {
    PlayerInst* player = gs->local_player();
    if (!player) {
        return;
    }
    EnemyInst* target = (EnemyInst*)gs->get_instance(player->target());
    if (!target) {
        return;
    }

    ldraw::draw_circle_outline(COL_GREEN.alpha(140),
            on_screen(gs, target->ipos()), target->target_radius + 5, 2);
}

void MonsterController::clear() {
    mids.clear();
    astarcontext = AStarPath();
}
