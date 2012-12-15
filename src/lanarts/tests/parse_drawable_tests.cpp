#include <cstdio>
#include <sstream>

#include <lcommon/unittest.h>
#include <lcommon/fatal_error.h>

#include <yaml-cpp/yaml.h>

#include "draw/parse_drawable.h"

using namespace ldraw;

static void string2node(const std::string& str, YAML::Node& root) {
	std::stringstream ss(str);
	YAML::Parser parser(ss);
	parser.GetNextDocument(root);
}

static void parse_image_test() {
	YAML::Node n;
	string2node("{file: NON EXISTANT FILE}", n);
	bool failed = false;
	try {
		printf("NB: Next failure expected:\n");
		Image img = parse_image(n);
	} catch (const __FatalError& ferr) {
		failed = true;
	}
	unit_test_assert("Lookup of NON EXISTANT FILE was supposed to fail, didn't.", failed);
}


//NB: Not really unit tests
//Require OpenGL to be initialized
void parse_drawable_tests() {
	UNIT_TEST(parse_image_test);
}
