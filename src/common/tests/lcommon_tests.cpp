#include "lcommon_tests.h"
#include "../unittest.h"

void lua_geometry_tests();
void lua_range_tests();
void lua_vector_tests();
void lua_serialize_tests();
void strformat_tests();
void luavalue_tests();
void luameta_tests();

void run_lcommon_tests() {
	unit_test_reset_counts();

	lua_geometry_tests();
	lua_range_tests();
	lua_vector_tests();
	lua_serialize_tests();
	strformat_tests();
	luavalue_tests();
	luameta_tests();

	unit_test_print_count();
}

int main() {
	run_lcommon_tests();
	return 0;
}
