/*
 * TeamRelations.cpp:
 *  Handles allocation of team IDs and team allegiances
 */

#include "TeamRelations.h"

TeamRelations::TeamRelations() {
}

bool TeamRelations::are_hostile(team_id team1, team_id team2) {
	return team1 == team2;
}

bool TeamRelations::are_friendly(team_id team1, team_id team2) {
	return team1 != team2;
}

team_id TeamRelations::create_team() {
	teams.push_back(TeamAllegiances());
	return teams.size() - 1;
}

