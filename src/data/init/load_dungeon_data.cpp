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

ItemGenSettings parse_item_gen(const YAML::Node& n) {
	Range amnt = parse_range(n["amount"]);

	vector<ItemGenChance> chances;
	if (hasnode(n, "generated")) {
		const YAML::Node& cnodes = n["generated"];
		chances.reserve(cnodes.size());
		for (int i = 0; i < cnodes.size(); i++) {
			const YAML::Node& cn = cnodes[i];

			ItemGenChance igc;
			string itemname;
			cn["item"] >> itemname;
			igc.itemtype = get_item_by_name(itemname.c_str());
			cn["chance"] >> igc.genchance;
			igc.quantity = parse_defaulted(cn, "quantity", Range(1,1));
			chances.push_back(igc);
		}
	}
	return ItemGenSettings(chances, amnt.min, amnt.max);
}
RoomGenSettings parse_room_gen(const YAML::Node& n) {
	int pad;
	n["padding"] >> pad;
	Range amount = parse_range(n["amount"]);
	Range size = parse_range(n["size"]);
	return RoomGenSettings(pad, amount.min, size.min, size.max);
}
TunnelGenSettings parse_tunnel_gen(const YAML::Node& n) {
	Range width = parse_range(n["width"]);
	Range per_room = parse_range(n["per_room"]);

	return TunnelGenSettings(parse_defaulted(n, "padding", 1),
			width.min, width.max,
			per_room.min, per_room.max);
}
FeatureGenSettings parse_feature_gen(const YAML::Node& n) {
	int nstairsup = parse_defaulted(n, "stairs_up", 3);
	int nstairsdown = parse_defaulted(n, "stairs_down", 3);
	int tileset = 0;
	if (hasnode(n, "tileset")){
		std::string tilesetname;
		n["tileset"] >> tilesetname;
		tileset = get_tileset_by_name(tilesetname.c_str());
	}
	return FeatureGenSettings(nstairsup, nstairsdown, tileset);
}

EnemyGenSettings parse_enemy_gen(const YAML::Node& supernode, const char* subnode) {
	vector<EnemyGenChance> chances;
	Range nmonsters;
	nmonsters.max = 0;
	nmonsters.min = 0;

	if (hasnode(supernode, subnode)){
		const YAML::Node& n = supernode[subnode];
		nmonsters = parse_range(n["amount"]);
		if (hasnode(n, "index")) {
			Range index = parse_range(n["index"]);
			int size = index.max - index.min + 1;
			for (int i = 0; i < size; i++) {
				EnemyGenChance egc;
				egc.enemytype = i + index.min;
				egc.genchance = 100 / size;
				egc.groupchance = 0;
				egc.groupsize = Range(0,0);
				chances.push_back(egc);
			}
		} else if (hasnode(n, "generated")) {
			const YAML::Node& cnodes = n["generated"];
			for (int i = 0; i < cnodes.size(); i++) {
				const YAML::Node& cn = cnodes[i];

				EnemyGenChance egc;
				egc.enemytype = parse_enemy_number(cn, "enemy");
				cn["chance"] >> egc.genchance;
				egc.groupchance = parse_defaulted(cn, "group_chance", 0);
				egc.groupsize = parse_defaulted(cn, "group_size", Range(0,0));

				chances.push_back(egc);
			}
		}
	}
	return EnemyGenSettings(chances, nmonsters.min, nmonsters.max);
}
LevelGenSettings parse_level_gen(const YAML::Node& n) {
	Range dim = parse_range(n["size"]);
	ItemGenSettings items = parse_item_gen(n["items"]);
	RoomGenSettings rooms = parse_room_gen(n["rooms"]);
	TunnelGenSettings tunnels = parse_tunnel_gen(n["tunnels"]);
	FeatureGenSettings features =
			hasnode(n, "features") ?
					parse_feature_gen(n["features"]) : featuredefault;
	EnemyGenSettings enemies = parse_enemy_gen(n, "enemies");

	return LevelGenSettings(dim.min, dim.max, items, rooms, tunnels, features,
			enemies);
}
void parse_dungeon_branch(const YAML::Node& n,
		std::vector<LevelGenSettings>& levels) {
	const YAML::Node& lnodes = n["levels"];
	for (int i = 0; i < lnodes.size(); i++) {
		//printf("Loading level\n");
		levels.push_back(parse_level_gen(lnodes[i]));
	}
}


void load_dungeon_callbackf(const YAML::Node& node, lua_State* L, LuaValue* value){
//	game_class_data.push_back( parse_class(node) );
}


LuaValue load_dungeon_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;
	try {

		std::string fname = "res/data/"+filenames[0];
		fstream file(fname.c_str(), fstream::in | fstream::binary);

		YAML::Parser parser(file);
		YAML::Node root;

		parser.GetNextDocument(root);

		const YAML::Node& node = root["branches"];
		//First branch should be main branch, using node[0]:
		parse_dungeon_branch(node[0], game_dungeon_yaml);

		game_dungeon_data[0] = DungeonBranch(&game_dungeon_yaml[0],
				game_dungeon_yaml.size());
		file.close();
	} catch (const YAML::Exception& parse) {
		printf("Dungeon Parsed Incorrectly: \n");
		printf("%s\n", parse.what());
	}
	return ret;
}
