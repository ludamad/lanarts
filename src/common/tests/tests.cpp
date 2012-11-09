#include "tests.h"
#include "../unittest.h"

void lua_binding_tests();

void run_lcommon_tests() {
	unit_test_reset_counts();

	lua_binding_tests();

	unit_test_print_count();
}


int main() {
	run_lcommon_tests();
	return 0;
}
