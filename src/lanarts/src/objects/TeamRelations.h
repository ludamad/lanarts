/*
 * TeamRelations.h:
 *  Handles allocation of team IDs and team allegiances
 */

#ifndef TEAMRELATIONS_H_
#define TEAMRELATIONS_H_

#include <vector>
#include "../lanarts_defines.h"

class CombatGameInst;

class TeamRelations {
public:
	TeamRelations();

	bool are_hostile(team_id team1, team_id team2);
	bool are_friendly(team_id team1, team_id team2);

	// Use these until we have good reason to create other teams
	team_id default_player_team();
	team_id default_enemy_team();

	team_id create_team(bool default_as_hostile = true,
			const std::vector<team_id>& exceptions = std::vector<team_id>());

private:
	struct TeamAllegiances {
		bool default_as_hostile;
		std::vector<team_id> exceptions;
		TeamAllegiances(bool default_as_hostile,
				const std::vector<team_id>& exceptions) :
				default_as_hostile(default_as_hostile), exceptions(exceptions) {
		}
	};
	team_id player_team, enemy_team;

	std::vector<TeamAllegiances> teams;
};

bool insts_are_hostile(TeamRelations& teams, CombatGameInst* inst1, CombatGameInst* inst2);


#endif /* TEAMRELATIONS_H_ */
