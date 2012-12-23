#include <cstdio>
#include <stdexcept>

#include <lcommon/unittest.h>

void lua_unit_tests();
void net_unit_tests();
void stat_serialize_tests();
void adv_net_unit_tests();
void parse_unit_tests();
void parse_drawable_tests();

void run_unit_tests() {
	unit_test_reset_counts();

	lua_unit_tests();
	net_unit_tests();
	stat_serialize_tests();
	adv_net_unit_tests();
	parse_unit_tests();
	parse_drawable_tests();

	unit_test_print_count();
}


int main(int argc, char** argv) {
	run_unit_tests();
	return 0;
}
