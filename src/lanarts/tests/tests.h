#ifndef TESTS_H_
#define TESTS_H_

#include <string>

typedef void (*unit_test_function)();

void unit_test_fail(const std::string& msg);
void unit_test_assert(const std::string& msg, bool condition);

#define UNIT_TEST_ASSERT(condition) unit_test_assert( #condition, condition )

bool unit_test(unit_test_function func, const char* fname);
void run_unit_tests();

#define UNIT_TEST(func) unit_test(func, #func )

#endif /* TESTS_H_ */
