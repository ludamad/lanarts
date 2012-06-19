/*
 * load_dungeon_data.cpp:
 *  Load dungeon area & level data
 */

#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>
#include "yaml_util.h"

using namespace std;

ItemGenChance parse_item_chance(const YAML::Node& n) {
	ItemGenChance igc;
	string itemname;
	n["item"] >> itemname;
	igc.itemtype = get_item_by_name(itemname.c_str());
	n["chance"] >> igc.genchance;
	igc.quantity = parse_defaulted(n, "quantity", Range(1, 1));
	return igc;
}
ItemGenSettings parse_item_gen(const YAML::Node& n) {
	ItemGenSettings igs;
	igs.num_items = parse_range(n["amount"]);

	if (yaml_has_node(n, "generated")) {
		igs.item_chances = parse_named_with_defaults(n["generated"], "item",
				&parse_item_chance);
	}
	return igs;
}
RoomGenSettings parse_room_gen(const YAML::Node& n) {
	RoomGenSettings rgs;
	rgs.amount_of_rooms = parse_range(n["amount"]);
	rgs.room_padding = parse_defaulted(n, "padding", 1);
	rgs.size = parse_range(n["size"]);
	return rgs;
}
TunnelGenSettings parse_tunnel_gen(const YAML::Node& n) {
	TunnelGenSettings tgs;
	tgs.size = parse_range(n["width"]);
	tgs.num_tunnels = parse_range(n["per_room"]);
	tgs.padding = parse_defaulted(n, "padding", 1);

	return tgs;
}
FeatureGenSettings parse_feature_gen(const YAML::Node& n) {
	FeatureGenSettings fgs;
	fgs.nstairs_up = parse_defaulted(n, "stairs_up", 3);
	fgs.nstairs_down = parse_defaulted(n, "stairs_down", 3);
	if (yaml_has_node(n, "tileset")) {
		std::string tilesetname;
		n["tileset"] >> tilesetname;
		fgs.tileset = get_tileset_by_name(tilesetname.c_str());
	}
	return fgs;
}

EnemyGenChance parse_enemy_chance(const YAML::Node& n) {
	EnemyGenChance egc;
	egc.enemytype = parse_enemy_number(n, "enemy");
	egc.genchance = parse_defaulted(n, "chance", 0);
	egc.guaranteed = parse_defaulted(n, "guaranteed_spawns", false);
	egc.groupchance = parse_defaulted(n, "group_chance", 0);
	egc.groupsize = parse_defaulted(n, "group_size", Range(0, 0));
	return egc;
}
EnemyGenSettings parse_enemy_gen(const YAML::Node& node, const char* key) {
	EnemyGenSettings egs;
	if (yaml_has_node(node, key)) {
		const YAML::Node& n = node[key];
		egs.wandering = parse_defaulted(n, "wandering", true);

		egs.num_monsters = parse_range(n["amount"]);
		if (yaml_has_node(n, "generated")) {
			egs.enemy_chances = parse_named_with_defaults(n["generated"],
					"enemy", &parse_enemy_chance);
		}
	}
	return egs;
}

ContentGenSettings parse_content_gen(const YAML::Node& n) {
	ContentGenSettings cgs;
	cgs.items = parse_item_gen(n["items"]);
	cgs.features = parse_feature_gen(n["features"]);
	cgs.enemies = parse_enemy_gen(n, "enemies");
	return cgs;
}

LayoutGenSettings parse_layout_gen(const YAML::Node& n) {
	LayoutGenSettings lgs;

	lgs.solid_fill = parse_defaulted(n, "solid_fill", true);
	lgs.tunnels = parse_tunnel_gen(n["tunnels"]);
	const YAML::Node& size = n["size"];
	lgs.width = parse_range(size[0]);
	lgs.height = parse_range(size[1]);

	const YAML::Node& rooms = n["rooms"];
	if (rooms.Type() == YAML::NodeType::Sequence) {
		for (int i = 0; i < rooms.size(); i++) {
			lgs.rooms.push_back(parse_room_gen(rooms[i]));
		}
	} else {
		lgs.rooms.push_back(parse_room_gen(rooms));
	}

	return lgs;
}

LevelGenSettings parse_level_gen(const YAML::Node& n) {
	LevelGenSettings level;
	level.content = parse_content_gen(n["content"]);

	const YAML::Node& layouts = n["layout"];
	if (layouts.Type() == YAML::NodeType::Sequence) {
		for (int i = 0; i < layouts.size(); i++) {
			level.layouts.push_back(parse_layout_gen(layouts[i]));
		}
	} else {
		level.layouts.push_back(parse_layout_gen(layouts));
	}
	return level;

}
void parse_dungeon_branch(const YAML::Node& n,
		std::vector<LevelGenSettings>& levels) {
	const YAML::Node& lnodes = n["levels"];
	for (int i = 0; i < lnodes.size(); i++) {
		//printf("Loading level\n");
		levels.push_back(parse_level_gen(lnodes[i]));
	}
}

void load_dungeon_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
//	game_class_data.push_back( parse_class(node) );
}

LuaValue load_dungeon_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;
	try {

		std::string fname = "res/data/" + filenames[0];
		fstream file(fname.c_str(), fstream::in | fstream::binary);

		YAML::Parser parser(file);
		YAML::Node root;

		parser.GetNextDocument(root);

		const YAML::Node& node = root["areas"];
		//First branch should be main branch, using node[0]:
		parse_dungeon_branch(node[0], game_dungeon_yaml);

		game_dungeon_data[0] = DungeonBranch(&game_dungeon_yaml[0],
				game_dungeon_yaml.size());
		file.close();
	} catch (const YAML::Exception& parse) {
		printf("Dungeon Parsed Incorrectly: \n");
		printf("%s\n", parse.what());
		fflush(stdout);
	}
	return ret;
}
