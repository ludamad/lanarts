/* unittest.h:
 *  Very simple framework for unit-tests.
 */

#ifndef UNITTEST_H_
#define UNITTEST_H_

#include <UnitTest++.h>
#include "luawrap/testutils.h"

#include <string>

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define FILE_AND_LINE __FILE__ " line " STRINGIZE(__LINE__)

typedef void (*unit_test_function)();

void unit_test_fail(const std::string& msg);

void unit_test_assert(const std::string& msg, bool condition);

#define UNIT_TEST_ASSERT(condition) unit_test_assert(FILE_AND_LINE "...\n\t" #condition, condition )

bool unit_test(unit_test_function func, const char* fname);
#define UNIT_TEST(func) unit_test(func, #func )
bool unit_test_suite(unit_test_function func, const char* fname);
#define UNIT_TEST_SUITE(func) unit_test_suite(func, #func )

void unit_test_reset_counts();
void unit_test_print_count();
bool unit_test_has_failure();

int run_unittests();

#endif /* UNITTEST_H_ */
