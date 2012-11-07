/* unittest.h:
 *  Very simple framework for unit-tests.
 */

#ifndef UNITTEST_H_
#define UNITTEST_H_

#include <string>

typedef void (*unit_test_function)();

void unit_test_fail(const std::string& msg);

void unit_test_assert(const std::string& msg, bool condition);
#define UNIT_TEST_ASSERT(condition) unit_test_assert( #condition, condition )

bool unit_test(unit_test_function func, const char* fname);
#define UNIT_TEST(func) unit_test(func, #func )

void unit_test_reset_counts();
void unit_test_print_count();

#endif /* UNITTEST_H_ */
