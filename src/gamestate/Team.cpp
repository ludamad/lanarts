#include <cstdlib>
#include <algorithm>
#include <lcommon/SerializeBuffer.h>
#include <limits>
#include "Team.h"
#include "TeamIter.h"
#include "gamestate/GameState.h"
#include "objects/CombatGameInst.h"
#include "objects/PlayerInst.h"

static constexpr float STRIDE = 15;
static constexpr float STRIDE_SQR = STRIDE * STRIDE;
static constexpr float MAX_DISTANCE_NPC_AGGRO_DISTANCE = 7 * TILE_SIZE;
static constexpr float MAX_DISTANCE_NPC_AGGRO_DISTANCE_SQR = (MAX_DISTANCE_NPC_AGGRO_DISTANCE * MAX_DISTANCE_NPC_AGGRO_DISTANCE);


static PosF get_direction_towards(GameState* gs, PosF pos, int obj_radius, BBox tile_span) {
    PosF direction_total = {0,0};
    float n_directions = 0.0f;
    // TODO use real interpolation? Lots of twiddly code though. For now, battletest this.
    FOR_EACH_BBOX(tile_span, tx, ty) {
        PosF target_pos = {tx * TILE_SIZE + TILE_SIZE/2, ty * TILE_SIZE + TILE_SIZE/2};
        PosF heading = target_pos - pos;
        if (square_magnitude(heading) > MAX_DISTANCE_NPC_AGGRO_DISTANCE_SQR) {
            continue;
        }
        // Get the accumulation vector:
        normalize(heading.x, heading.y, STRIDE);
        PosF accum = pos;
        // Accumulate until we have reached our target, or hit a wall:
        bool reached = true;
        while (squared_distance(accum, target_pos) < STRIDE_SQR) {
            if (gs->tile_radius_test(accum.x, accum.y, obj_radius)) {
                // We hit a wall, make sure to not count this towards accumulation.
                //reached = false;
                //break;
            }
            accum += heading;
        }
        if (!reached) {
            // Hit a wall:
            continue;
        }
        // We have reached our target, add to our direction total:
        direction_total += heading;

    }
    return direction_total.scaled(n_directions == 0.0f ? 1.0f : 1.0f / n_directions);
}

static BBox to_tile_span(GameState* gs, BBox area) {
    int w = gs->width() / TILE_SIZE, h = gs->height() / TILE_SIZE;
    int mingrid_x = area.x1 / TILE_SIZE, mingrid_y = area.y1 / TILE_SIZE;
    int maxgrid_x = area.x2 / TILE_SIZE, maxgrid_y = area.y2 / TILE_SIZE;
    int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
    int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);
    return {minx, miny, maxx, maxy};
}

CombatGameInst* get_nearest_visible_enemy(GameState* gs, CombatGameInst* inst) {
    float smallest_sqr_dist = std::numeric_limits<float>::max();
    CombatGameInst* closest_game_inst = NULL;
    for_all_enemies(gs->team_data(), inst, [&](CombatGameInst* o) {
            if (!is_visible(gs, inst, o)) {
                return;
            }
            float dx = (o->x - inst->x), dy = (o->y - inst->y);
            float sqr_dist = dx*dx + dy*dy;
            if (sqr_dist < smallest_sqr_dist) {
                smallest_sqr_dist = dx*dx + dy*dy;
                closest_game_inst = o;
            }
        }
    );
    return closest_game_inst;
}


CombatGameInst* get_nearest_enemy(GameState* gs, CombatGameInst* inst) {
    float smallest_sqr_dist = std::numeric_limits<float>::max();
    CombatGameInst* closest_game_inst = NULL;
    for_all_enemies(gs->team_data(), inst, [&](CombatGameInst* o) {
            float dx = (o->x - inst->x), dy = (o->y - inst->y);
            float sqr_dist = dx*dx + dy*dy;
            if (sqr_dist < smallest_sqr_dist) {
                smallest_sqr_dist = dx*dx + dy*dy;
                closest_game_inst = o;
            }
        }
    );
    return dynamic_cast<CombatGameInst*>(closest_game_inst);
}

CombatGameInst* get_nearest_ally(GameState* gs, CombatGameInst* inst) {
    float smallest_sqr_dist = std::numeric_limits<float>::max();
    CombatGameInst* closest_game_inst = NULL;
    for_all_allies(gs->team_data(), inst, [&](CombatGameInst* o) {
            if (inst == o) {
                return;
            }
            float dx = (o->x - inst->x), dy = (o->y - inst->y);
            float sqr_dist = dx*dx + dy*dy;
            if (sqr_dist < smallest_sqr_dist) {
                smallest_sqr_dist = dx*dx + dy*dy;
                closest_game_inst = o;
            }
        }
    );
    return dynamic_cast<CombatGameInst*>(closest_game_inst);
}

PosF get_direction_towards(GameState* gs, CombatGameInst* from,
        CombatGameInst* to, float max_speed) {
    if (to->has_paths_data()) {
        return to->paths_to_object().interpolated_direction(from->bbox(), max_speed);
    }
    PosF diff = get_direction_towards(gs, from->pos(), from->radius, to_tile_span(gs, to->bbox()));
    if (diff == PosF()) {
        return diff;
    }
    normalize(diff.x, diff.y, max_speed);
    return diff;
}

bool are_allies(CombatGameInst* inst1, CombatGameInst* inst2) {
    return inst1->team != inst2->team;
}

bool are_enemies(CombatGameInst* inst1, CombatGameInst* inst2) {
    return inst1->team != inst2->team;
}

static bool are_tiles_visible(GameState* gs, CombatGameInst* viewer, BBox tile_span) {
    fov* fov = viewer->field_of_view;
    if (!fov) {
        return true;
        // TODO think harder about this.
        PosF dir = get_direction_towards(gs, Pos {viewer->pos().scaled(1.0f/32.0f)}, viewer->radius, tile_span);
        bool visible = (dir != PosF());
        return visible;
    }
    return fov->within_fov(tile_span);
}

bool is_visible(GameState* gs, CombatGameInst* viewer, BBox area) {
    return are_tiles_visible(gs, viewer, to_tile_span(gs, area));
}

bool is_visible(GameState* gs, CombatGameInst* viewer,
        CombatGameInst* observed) {
    if (!viewer->field_of_view && observed->field_of_view)
        return is_visible(gs, observed, viewer);
    return is_visible(gs, viewer, observed->bbox());
}


void TeamData::remove(CombatGameInst* obj) {
//    _ensure(obj->current_floor, obj->team); // Should already exist
    auto& vec = teams.at(obj->team).per_level_data.at(obj->current_floor);
    auto pos = std::find(vec.begin(), vec.end(), obj);
    if (pos != vec.end()) {
        vec.erase(pos);
    }
}

void TeamData::add(CombatGameInst* obj) {
    _ensure(obj->current_floor, obj->team);
    teams.at(obj->team).per_level_data.at(obj->current_floor).push_back(obj);
}

void TeamData::_ensure(level_id level, team_id team) {
    _ensure(team);
    if (teams[team].per_level_data.size() <= level) {
        teams[team].per_level_data.resize(level + 1);
    }
}

void TeamData::_ensure(team_id team) {
    if (teams.size() <= team) {
        teams.resize(team + 1);
    }
}


// Serialization TEDIOUSNESS follows.
// TODO move to 'cereal' and cut down on this nonsense.

template <typename T>
static void write_inst_ref(T* ref, GameState* gs,
        SerializeBuffer& serializer) {
    serializer.write_int(ref->id);
    serializer.write_int(ref->current_floor);
}

template <typename T>
static void read_inst_ref(T*& ref, GameState* gs,
        SerializeBuffer& serializer) {
    int id, level;
    serializer.read_int(id);
    serializer.read_int(level);
    ref = dynamic_cast<T*>(gs->get_instance(level, id));
}

void TeamPlayerData::serialize(GameState* gs, SerializeBuffer& buffer) {
    buffer.write(this->gold);
    buffer.write_int(this->players.size());
    for (auto* p : this->players) {
        write_inst_ref(p, gs, buffer);
    }
}

void TeamPlayerData::deserialize(GameState* gs, SerializeBuffer& buffer) {
    buffer.read(this->gold);
    int size = buffer.read_int();
    this->players.clear();
    this->players.resize(size);
    for (auto*& p : this->players) {
        read_inst_ref(p, gs, buffer);
    }
}

void Team::serialize(GameState* gs, SerializeBuffer& buffer) {
    buffer.write_int(this->per_level_data.size());
    for (auto& data : this->per_level_data) {
        buffer.write_int(data.size());
        for (auto* inst : data) {
            write_inst_ref(inst, gs, buffer);
        }
    }
    this->player_data.serialize(gs, buffer);
}

void Team::deserialize(GameState* gs, SerializeBuffer& buffer) {
    int size = buffer.read_int();
    this->per_level_data.clear();
    this->per_level_data.resize(size);
    for (auto& data : this->per_level_data) {
        int data_size = buffer.read_int();
        data.clear();
        for (int i = 0; i < data_size; i++) {
            CombatGameInst* ref = NULL;
            read_inst_ref(ref, gs, buffer);
            data.push_back(ref);
        }
    }
    this->player_data.deserialize(gs, buffer);
}

void TeamData::serialize(GameState* gs, SerializeBuffer& buffer) {
    buffer.write_int(this->teams.size());
    for (auto& team : this->teams) {
        team.serialize(gs, buffer);
    }
}

void TeamData::deserialize(GameState* gs, SerializeBuffer& buffer) {
    this->teams.clear();
    this->teams.resize(buffer.read_int());
    for (auto& team : this->teams) {
        team.deserialize(gs, buffer);
    }
}
