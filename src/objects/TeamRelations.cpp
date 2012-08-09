/*
 * TeamRelations.cpp:
 *  Handles allocation of team IDs and team allegiances
 */

#include "TeamRelations.h"
#include <algorithm>

TeamRelations::TeamRelations() {
	player_team = create_team();
	enemy_team = create_team();
}

bool TeamRelations::are_hostile(team_id team1, team_id team2) {
	TeamAllegiances& allegiances = teams.at(team1);
	const std::vector<team_id>& except = allegiances.exceptions;
	bool is_exception = std::find(except.begin(), except.end(), team2)
			!= except.end();
	return (allegiances.default_as_hostile != is_exception);
}

bool TeamRelations::are_friendly(team_id team1, team_id team2) {
	return !are_hostile(team1, team2);
}

team_id TeamRelations::default_player_team() {
	return player_team;
}

team_id TeamRelations::default_enemy_team() {
	return enemy_team;
}

team_id TeamRelations::create_team(bool default_as_hostile,
		const std::vector<team_id> & exceptions) {
	team_id ret_id = teams.size();
	teams.push_back(TeamAllegiances(default_as_hostile, exceptions));
	for (int i = 0; i < exceptions.size(); i++) {
		TeamAllegiances& allegiances = teams.at(exceptions[i]);
		if (allegiances.default_as_hostile == default_as_hostile) {
			allegiances.exceptions.push_back(ret_id);
		}
	}
	return ret_id;
}

