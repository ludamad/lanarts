/*
 * parse_drawable.cpp:
 *  Parses arbitrary drawable objects (eg, image based or lua defined animations)
 */

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

#include <common/strformat.h>
#include <common/fatal_error.h>

#include "../data/FilenameList.h"
#include "../data/parse.h"
#include "../data/parse_context.h"

#include "parse_drawable.h"

using namespace ldraw;

Image parse_image(const YAML::Node& node) {
	const YAML::Node* filenode = yaml_find_node(node, "file");

	if (!filenode) {
		throw YAML::RepresentationException(node.GetMark(),
				"Expected list (eg [\"file.png\"]), or pattern (eg \"file(0-9).png\").");
	}

	return Image(parse_str(*filenode));
}

Animation parse_animation(const YAML::Node& node) {
	const YAML::Node* framenode = yaml_find_node(node, "frames");

	if (!framenode) {
		throw YAML::RepresentationException(node.GetMark(),
				"Expected 'frames:' for animation drawable object (has 'type: animation').");
	}

	return Animation(parse_drawable_list(*framenode));
}

LuaDrawable parse_luadrawable(const YAML::Node& node) {
	lua_State* L = ParseContext::luastate();

	const YAML::Node* funcnode = yaml_find_node(node, "file");

	if (!funcnode) {
		throw YAML::RepresentationException(node.GetMark(),
				"Expected 'function:' for lua drawable object (has 'type: lua').");
	}

	return LuaDrawable(L, parse_str(*funcnode));
}

DirectionalDrawable parse_directional(const YAML::Node& node) {
	const YAML::Node* framenode = yaml_find_node(node, "frames");

	if (!framenode) {
		throw YAML::RepresentationException(node.GetMark(),
				"Expected 'frames:' for directional drawable object (has 'type: directional').");
	}

	return DirectionalDrawable(parse_drawable_list(*framenode));
}

static bool parse_image_split_pattern(const std::string& pattern, Dim* size,
		std::string* filename) {
	int pos1 = pattern.find('%');

	if (pos1 == std::string::npos) {
		return false;
	}

	int pos2 = -1;
	int num_set = sscanf(&pattern[pos1 + 1], "%dx%d%n", &size->w, &size->h,
			&pos2);

	if (pos2 + pos1 + 1 != pattern.size()) {
		printf("Bad file pattern '%s', expected "
				"something like 'filename.png%%32x32'\n", pattern.c_str());
		fatal_error();
	}

	*filename = pattern.substr(0, pos1);

	return true;
}

bool filepattern_to_image_list(std::vector<Image>& images,
		const std::string& pattern) {
	// Try range pattern
	FilenameList files;
	bool was_range_pattern = filenames_from_pattern(files, pattern);

	if (was_range_pattern) {
		for (int i = 0; i < files.size(); i++) {
			images.push_back(Image(files[i]));
		}
		return true;
	}

	// Try split pattern
	Dim size;
	std::string filename;

	bool was_split_pattern = parse_image_split_pattern(pattern, &size,
			&filename);

	if (was_split_pattern) {
		// Split
		std::vector<Image> split_images = image_split(Image(filename), size);

		for (int i = 0; i < split_images.size(); i++) {
			images.push_back(split_images[i]);
		}
		return true;
	}

	// Treat as normal filename
	images.push_back(Image(pattern));

	return false;
}
//static bool filepattern_to_image_list(std::vector<Image>& images,
//		const YAML::Node& node) {
//	FilenameList files;
//	bool was_pattern = filenames_from_pattern(files, parse_str(node));
//
//	for (int i = 0; i < files.size(); i++) {
//		images.push_back(Image(files[i]));
//	}
//
//	return was_pattern;
//}

static void images_to_drawable_list(std::vector<Drawable>& drawable,
		const std::vector<Image>& images) {
	for (int i = 0; i < images.size(); i++) {
		drawable.push_back(Drawable(new Image(images[i])));
	}
}

// If node is a string, loads multiple images based on a pattern
// If node is a list, calls parse_drawable on individual entries
std::vector<Drawable> parse_drawable_list(const YAML::Node& node) {
	std::vector<Drawable> drawables;

	// Error if map
	if (node.Type() == YAML::NodeType::Map) {
		throw YAML::RepresentationException(node.GetMark(),
				"Expected list (eg [\"file.png\"]), or pattern (eg \"file(0-9).png\").");
	}

	// Interpret as file pattern if string
	if (node.Type() == YAML::NodeType::Scalar) {
		std::vector<Image> images;

		if (!filepattern_to_image_list(images, parse_str(node))) {
			throw YAML::RepresentationException(node.GetMark(),
					"Expected list (eg [\"file.png\"]), or pattern (eg \"file(0-9).png\").");
		}

		images_to_drawable_list(drawables, images);
		return drawables;
	}

	// Accumulate from list
	int size = node.size();
	for (int i = 0; i < size; i++) {
		const YAML::Node& child = node[i];

		// Expand any file patterns, or image file names
		if (child.Type() == YAML::NodeType::Scalar) {
			std::vector<Image> images;
			filepattern_to_image_list(images, parse_str(child));
			images_to_drawable_list(drawables, images);
		} else {
			parse_drawable(child);
		}
	}

	return drawables;
}

// If node is a string, loads as image
// If node is a map, 'type:' indicates drawable type
std::vector<Image> parse_image_list(const YAML::Node& node) {
	std::vector<Image> images;

	// Interpret as file pattern if string
	if (node.Type() == YAML::NodeType::Scalar) {
		if (!filepattern_to_image_list(images, parse_str(node))) {
			throw YAML::RepresentationException(node.GetMark(),
					"Expected list (eg [\"file.png\"]), or pattern (eg \"file(0-9).png\").");
		}
		return images;
	}

	// Accumulate from list
	int size = node.size();
	for (int i = 0; i < size; i++) {
		filepattern_to_image_list(images, parse_str(node[i]));
	}

	return images;
}

Drawable parse_drawable(const YAML::Node& node) {
	const char* err =
			"Expected drawable object, of form \n"
					"     type: image -or- animation -or- directional -or- lua \n"
					"     file: <filename>\n"
					"-or- frames: <drawable list or file pattern>\n"
					"         Note: file patterns are of the form filename(min-max).extension\n"
					"-or- function: <lua function>";

	if (node.Type() != YAML::NodeType::Map) {
		throw YAML::RepresentationException(node.GetMark(), err);
	}

	std::string type = parse_optional(node, "type", std::string());

	if ((type.empty() && yaml_has_node(node, "file")) || type == "image") {
		return Drawable(new Image(parse_image(node)));
	} else if (type == "animation") {
		return Drawable(new Animation(parse_animation(node)));
	} else if (type == "directional") {
		return Drawable(new DirectionalDrawable(parse_directional(node)));
	} else if (type == "directional") {
		return Drawable(new LuaDrawable(parse_luadrawable(node)));
	} else {
		throw YAML::RepresentationException(node.GetMark(), err);
	}

}
