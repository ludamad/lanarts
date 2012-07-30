/*
 * TeamRelations.h:
 *  Handles allocation of team IDs and team allegiances
 */

#ifndef TEAMRELATIONS_H_
#define TEAMRELATIONS_H_

#include "../lanarts_defines.h"

struct TeamAllegiances {
	std::vector<team_id> allied_teams;
};

class TeamRelations {
public:
	TeamRelations();

	bool are_hostile(team_id team1, team_id team2);
	bool are_friendly(team_id team1, team_id team2);

	team_id create_team();

private:
	std::vector<TeamAllegiances> teams;
};

#endif /* TEAMRELATIONS_H_ */
