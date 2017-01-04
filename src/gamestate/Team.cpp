#include <cstdlib>
#include <limits>
#include "Team.h"

fov* LevelTeamData::get_fov_of(GameState* gs, CombatGameInst* inst) {
    return _get_member_data_of(inst)->field_of_view;
}

CombatGameInst* LevelTeamData::get_nearest_enemy(GameState* gs, CombatGameInst* inst) {
    float smallest_sqr_dist = std::numeric_limits<float>::max();
    CombatGameInst* closest_game_inst = NULL;
    for (auto& team : _teams) {
        if (inst->team == team.id) {
            continue;
        }
        _get_nearest_on_team(gs, team, inst, &smallest_sqr_dist, &closest_game_inst);
    }
    return dynamic_cast<CombatGameInst*>(closest_game_inst);
}

CombatGameInst* LevelTeamData::get_nearest_ally(GameState* gs,
        CombatGameInst* inst) {
    float smallest_sqr_dist = std::numeric_limits<float>::max();
    CombatGameInst* closest_game_inst = NULL;
    _get_nearest_on_team(gs, *_get_team_of(inst), inst, &smallest_sqr_dist, &closest_game_inst);
    return dynamic_cast<CombatGameInst*>(closest_game_inst);
}

PosF LevelTeamData::get_direction_towards(GameState* gs, CombatGameInst* from,
        CombatGameInst* to, float max_speed) {
    if (to->is_major_character()) {
    }
}

bool LevelTeamData::are_allies(CombatGameInst* inst1, CombatGameInst* inst2) {
    return inst1->team != inst2->team;
}

bool LevelTeamData::are_enemies(CombatGameInst* inst1, CombatGameInst* inst2) {
    return inst1->team != inst2->team;
}

_TeamMemberData* LevelTeamData::_get_member_data_of(CombatGameInst* inst) {
    _Team* team = _get_team_of(inst);
    if (team) {
        return &team->members[inst->id];
    }
    LANARTS_ASSERT(false);
    return NULL;
}

_Team* LevelTeamData::_get_team_of(CombatGameInst* inst) {
    for (auto& team : _teams) {
        if (inst->team == team.id) {
            return &team;
        }
    }
    return NULL;
}

bool LevelTeamData::are_tiles_visible(GameState* gs, CombatGameInst* viewer, BBox tile_span) {
    fov* fov = get_fov_of(gs, viewer);
    if (!fov) {
        // For now:
        return true;
    }
    return fov->within_fov(tile_span);
}

static BBox to_tile_span(GameState* gs, BBox area) {
    int w = gs->width() / TILE_SIZE, h = gs->height() / TILE_SIZE;
    int mingrid_x = area.x1 / TILE_SIZE, mingrid_y = area.y1 / TILE_SIZE;
    int maxgrid_x = area.x2 / TILE_SIZE, maxgrid_y = area.y2 / TILE_SIZE;
    int minx = squish(mingrid_x, 0, w), miny = squish(mingrid_y, 0, h);
    int maxx = squish(maxgrid_x, 0, w), maxy = squish(maxgrid_y, 0, h);
    return {minx, miny, maxx, maxy};
}

bool LevelTeamData::is_visible(GameState* gs, CombatGameInst* viewer, BBox area) {
    return are_tiles_visible(gs, viewer, to_tile_span(gs, area));
}

bool LevelTeamData::is_visible(GameState* gs, CombatGameInst* viewer,
        CombatGameInst* observed) {
    return is_visible(gs, viewer, observed->bbox());
}

void LevelTeamData::_get_nearest_on_team(GameState* gs, _Team& team, CombatGameInst* inst, float* smallest_sqr_dist,
        CombatGameInst** closest_game_inst) {
    GameInst* closest = *closest_game_inst;
    for (auto& entry : team.members) {
        GameInst* o = gs->get_instance(entry.first);
        LANARTS_ASSERT(dynamic_cast<CombatGameInst*>(o));
        float dx = (o->x - inst->x), dy = (o->y - inst->y);
        float sqr_dist = dx*dx + dy*dy;
        if (sqr_dist < *smallest_sqr_dist) {
            *smallest_sqr_dist = dx*dx + dy*dy;
            closest = o;
        }
    }
    *closest_game_inst = dynamic_cast<CombatGameInst*>(closest);
}

void LevelTeamData::remove_instance(GameState* gs, CombatGameInst* inst) {
    _Team* team = _get_team_of(inst);
    LANARTS_ASSERT(team != NULL);
    team->members.erase(inst->id);
}

void LevelTeamData::add_instance(GameState* gs, CombatGameInst* inst) {
    _Team* team = _get_team_of(inst);
    if (team == NULL) {
        _teams.push_back({inst->team, {}});
        team = &_teams.back();
    }
    auto& data = team->members[inst->id];
    LANARTS_ASSERT(data.field_of_view == NULL);
    if (inst->is_major_character()) {
        data.field_of_view = new fov(inst->vision_radius);
        data.paths_to_object.initialize(gs->tiles().solidity_map());
        data.paths_to_object.fill_paths_in_radius(inst->ipos(), PLAYER_PATHING_RADIUS);
    }
}
