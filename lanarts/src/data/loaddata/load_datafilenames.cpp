/*
 * load_datafilenames.cpp:
 *  Loads data file locations from res/datafiles.yaml
 */

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "../game_data.h"
#include "../yaml_util.h"

using namespace std;

DataFiles load_datafilenames(const char* filename) {
	DataFiles dfiles;

	fstream file(filename, fstream::in | fstream::binary);

	if (!file) {
		fprintf(
				stderr,
				"Fatal error: file %s not found, ensure you are running from directory with res/ folder\n.",
				filename);
		exit(0);
	}

	if (file) {
		try {
			YAML::Parser parser(file);
			YAML::Node root;

			parser.GetNextDocument(root);

			optional_set(root, "equipment_files", dfiles.equipment_files);
			optional_set(root, "class_files", dfiles.class_files);
			optional_set(root, "enemy_files", dfiles.enemy_files);
			optional_set(root, "effect_files", dfiles.effect_files);
			optional_set(root, "item_files", dfiles.item_files);
			optional_set(root, "itemgenlist_files", dfiles.itemgenlist_files);
			optional_set(root, "map_files", dfiles.map_files);
			optional_set(root, "room_template_files", dfiles.room_template_files);
			optional_set(root, "projectile_files", dfiles.projectile_files);
			optional_set(root, "spell_files", dfiles.spell_files);
			optional_set(root, "sprite_files", dfiles.sprite_files);
			optional_set(root, "tile_files", dfiles.tile_files);
			optional_set(root, "tileset_files", dfiles.tileset_files);
			optional_set(root, "weapon_files", dfiles.weapon_files);

		} catch (const YAML::Exception& parse) {
			printf("data file lists parsed incorrectly: \n");
			printf("%s\n", parse.what());
		}
	}

	return dfiles;
}

