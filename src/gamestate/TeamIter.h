#ifndef TEAM_ITER_H_
#define TEAM_ITER_H_

#include "Team.h"
#include "objects/CombatGameInst.h"

// Helper functions for iterating through TeamData:
template <typename Function>
inline void for_all_enemies(TeamData& td, level_id level, team_id team, Function&& f) {
    //td._ensure(level, team);
    LANARTS_ASSERT(team >= 0 && team < td.teams.size());
    for (int i = 0; i < td.teams.size(); i++) {
        if (i == team) {
            continue; // Skip own team
        }
        LANARTS_ASSERT(level >= 0 && level < td.teams[i].per_level_data.size());
        auto& objs = td.teams[i].per_level_data[level];
        for (CombatGameInst* e : objs) {
            f(e);
        }
    }
}
// Helper functions for iterating through TeamData:
template <typename Function>
inline void for_all_enemies(TeamData& td, CombatGameInst* inst, Function&& f) {
    for_all_enemies(td, inst->current_floor, inst->team, f);
}

template <typename Function>
inline void for_all_on_team(TeamData& td, level_id level, team_id team, Function&& f) {
    td._ensure(level, team);
    LANARTS_ASSERT(team >= 0 && team < td.teams.size());
    auto& objs = td.teams[team].per_level_data[level];
    for (CombatGameInst* e : objs) {
        f(e);
    }
}
// Helper functions for iterating through TeamData:
template <typename Function>
inline void for_all_allies(TeamData& td, CombatGameInst* inst, Function&& f) {
    for_all_on_team(td, inst->current_floor, inst->team, f);
}


template <typename Function>
inline void for_players_on_team(TeamData& td, team_id team, Function&& f) {
    td._ensure(team);
    LANARTS_ASSERT(team >= 0 && team < td.teams.size());
    auto& objs = td.teams[team].player_data.players;
    for (PlayerInst* e : objs) {
        f(e);
    }
}

#endif
