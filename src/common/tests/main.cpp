//#include "lcommon_tests.h"
//#include "unittest.h"
//
//void lua_geometry_tests();
//void lua_range_tests();
//void lua_serialize_tests();
//void strformat_tests();
//void luavalue_tests();
//
//void run_lcommon_tests() {
//	unit_test_reset_counts();
//
//	UNIT_TEST_SUITE(lua_geometry_tests);
//	UNIT_TEST_SUITE(lua_range_tests);
//	UNIT_TEST_SUITE(lua_serialize_tests);
//	UNIT_TEST_SUITE(strformat_tests);
//	UNIT_TEST_SUITE(luavalue_tests);
//
//	unit_test_print_count();
//}
//
//int main(int argc, char** argv) {
//	run_lcommon_tests();
//	return unit_test_has_failure();
//}

#include <cstdio>

#include <stdexcept>

#include "unittest.h"

int main(int argc, char** argv) {
	return run_unittests();
}
