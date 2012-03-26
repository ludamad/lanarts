/*
 * load_dungeon_data.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: 100397561
 */


#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>
#include "yaml_util.h"

using namespace std;

FeatureGenSettings featuredefault(3, 3 /*3 stairs up, 3 stairs down*/,
		1 /*Default tileset*/);

ItemGenSettings parse_item_gen(const YAML::Node& n){
	GenRange amnt = parse_range(n["amount"]);
	return ItemGenSettings(1, amnt.min, amnt.max);
}
RoomGenSettings parse_room_gen(const YAML::Node& n){
	int pad;
	n["padding"] >> pad;
	GenRange amount = parse_range(n["amount"]);
	GenRange size = parse_range(n["size"]);
	return RoomGenSettings(pad, amount.min, size.min, size.max);
}
TunnelGenSettings parse_tunnel_gen(const YAML::Node& n){
	GenRange width = parse_range(n["width"]);
	GenRange per_room = parse_range(n["per_room"]);
	return TunnelGenSettings(width.min, width.max, per_room.min, per_room.max);
}
FeatureGenSettings parse_feature_gen(const YAML::Node& n){
	int nstairsup = parse_defaulted(n, "stairs_up", 3);
	int nstairsdown = parse_defaulted(n, "stairs_down", 3);
	return FeatureGenSettings(nstairsup, nstairsdown, 1);
}
EnemyGenSettings parse_enemy_gen(const YAML::Node& n){
	GenRange index = parse_range(n["index"]);
	GenRange nmonsters = parse_range(n["amount"]);

	return EnemyGenSettings(index.min, index.max, nmonsters.min);
}
LevelGenSettings parse_level_gen(const YAML::Node& n){
	GenRange dim = parse_range(n["size"]);
	ItemGenSettings items = parse_item_gen(n["items"]);
	RoomGenSettings rooms = parse_room_gen(n["rooms"]);
	TunnelGenSettings tunnels = parse_tunnel_gen(n["tunnels"]);
	FeatureGenSettings features = hasnode(n, "features") ?
			parse_feature_gen(n["features"]) : featuredefault;
	EnemyGenSettings enemies = parse_enemy_gen(n["enemies"]);

	return LevelGenSettings(dim.min, dim.max, items, rooms, tunnels, features, enemies);
}
void parse_dungeon_branch(const YAML::Node& n, std::vector<LevelGenSettings>& levels){
	const YAML::Node& lnodes = n["levels"];
	for (int i = 0; i < lnodes.size(); i++){
		levels.push_back(parse_level_gen(lnodes[i]));
	}
}

void load_dungeon_data(const char* filename){
	try {

		fstream file(filename, fstream::in | fstream::binary);

		YAML::Parser parser(file);
		YAML::Node root;

		parser.GetNextDocument(root);

		const YAML::Node& node = root["branches"];
		//First branch should be main branch, using node[0]:
		parse_dungeon_branch(node[0], game_dungeon_yaml);

		game_dungeon_data[0] = DungeonBranch(&game_dungeon_yaml[0], game_dungeon_yaml.size());
	} catch (const YAML::Exception& parse){
		printf("Dungeon Parsed Incorrectly: \n");
		printf("%s\n", parse.what());
	}
}
