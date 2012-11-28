/*
 * parse_drawable.cpp:
 *  Parses arbitrary drawable objects (eg, image based or lua defined animations)
 */

#include <yaml-cpp/yaml.h>

#include <common/strformat.h>

#include "../data/FilenameList.h"

#include "parse_drawable.h"

using namespace ldraw;

Image parse_image(const YAML::Node& node) {
	std::string s;
	node >> s;
	return Image(s);
}

//Animation parse_animation(const YAML::Node& node) {
//	std::string type;
//	node["type"] >> type;
//	Animation animation(parse_drawable_list(node["frames"]));
////animation.set_animation_duration();
//}
//
//LuaDrawable parse_luadrawable(const YAML::Node& node) {
//}
//
//DirectionalDrawable parse_directional(const YAML::Node& node) {
//}
//
// If node is a string, loads multiple images based on a pattern
// If node is a list, calls parse_drawable on individual entries
std::vector<Drawable> parse_drawable_list(const YAML::Node& node) {
	std::vector<Drawable> drawables;
	return drawables;
}
//
//// If node is a string, loads as image
//// If node is a map, 'type:' indicates drawable type
//std::vector<Image> parse_image_list(const YAML::Node& node) {
//}
//
//Drawable parse_drawable(const YAML::Node& node) {
//}

