#include <common/unittest.h>
#include "ldraw_tests.h"

void lua_colour_tests();
void lua_image_tests();
void lua_drawoptions_tests();

void run_ldraw_tests() {
	unit_test_reset_counts();

	lua_colour_tests();
	lua_image_tests();
	lua_drawoptions_tests();

	unit_test_print_count();
}

int main() {
	run_ldraw_tests();
	return 0;
}
