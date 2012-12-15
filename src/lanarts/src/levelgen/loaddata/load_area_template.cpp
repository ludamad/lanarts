/*
 * load_area_template.cpp:
 *  Load predefined area generation information
 */

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "data/FilenameList.h"
#include "data/parse.h"
#include "data/yaml_util.h"

#include "objects/enemy/EnemyEntry.h"

static std::vector<Glyph> load_template_legend(const YAML::Node& root,
		const char* key) {
	std::vector<Glyph> glyphs;
	if (yaml_has_node(root, key)) {
		const YAML::Node& node = root[key];
		for (int i = 0; i < node.size(); i++) {
			const YAML::Node& n = node[i];
			std::string glyph_char = parse_str(n["glyph"]);
			LANARTS_ASSERT(glyph_char.size() == 1);
			std::string enemytype = parse_optional(n, "enemy", std::string());
			glyphs.push_back(
					Glyph(glyph_char.at(0),
							get_enemy_by_name(enemytype.c_str())));
		}
	}

	return glyphs;
}

static int strlinewidth(const std::string& data) {
	for (int i = 0; i < data.size(); i++) {
		if (isspace(data[i])) {
			return i;
		}
	}
	return data.size();
}

static int strlineheight(const std::string& data, int width) {
	int chars_since_space = 0;
	int height = 0;
	for (int i = 0; i < data.size(); i++) {
		if (isspace(data[i])) {
			//TODO: throw exception that won't be turned off in debug mode
			LANARTS_ASSERT(chars_since_space == width);
			chars_since_space = 0;
			do {
				i++;
			} while (isspace(data[i]));
			height++;
		}
		chars_since_space++;
	}
	return height;
}

static void load_area_template_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* _) {
	std::vector<Glyph> glyphs = load_template_legend(node, "legend");
	std::string name = parse_str(node["name"]), data = parse_str(node["data"]);
	int width = strlinewidth(data);
	int height = strlineheight(data, width);
	printf("w: %d, h:%d\n", width, height);
	LANARTS_ASSERT(width > 0 && height > 0);
	int data_width = width + 1;
	AreaTemplate* area_template = new AreaTemplate(name, data.c_str(),
			data_width, width, height, glyphs);
	add_area_template(area_template);

}

void load_area_template_data(const FilenameList& filenames) {
	clear_area_templates();
	load_data_impl_template(filenames, "templates",
			load_area_template_callbackf);
}
