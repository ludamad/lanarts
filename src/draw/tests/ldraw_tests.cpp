#include <common/unittest.h>
#include "ldraw_tests.h"

void lua_colour_tests();
void lua_image_tests();
void lua_drawoptions_tests();
void ldraw_image_tests();
void ldraw_animation_tests();
void ldraw_directionaldrawable_tests();
void lua_font_tests();
void lua_drawable_tests();

void run_ldraw_tests() {
	unit_test_reset_counts();

	lua_image_tests();
	lua_colour_tests();
	lua_drawoptions_tests();
	ldraw_image_tests();
	ldraw_animation_tests();
	ldraw_directionaldrawable_tests();
	lua_font_tests();
	lua_drawable_tests();

	unit_test_print_count();
}

int main() {
	run_ldraw_tests();
	return 0;
}
