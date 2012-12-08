#include "unittest.h"
#include <cstdio>
#include <stdexcept>
#include <cassert>

struct Failure {
	std::string msg;
	Failure(const std::string& msg) :
			msg(msg) {
	}
};

void unit_test_fail(const std::string& msg) {
	throw Failure(msg);
}

void unit_test_assert(const std::string& msg, bool condition) {
	if (!condition) {
		unit_test_fail(msg);
	}
}

static int unit_test_runs = 0, unit_test_failures = 0;
static int unit_test_suite_runs = 0, unit_test_suite_failures = 0;

bool unit_test(unit_test_function func, const char* fname) {
	unit_test_runs++;
	unit_test_suite_runs++;
	try {
		func();
		printf("** Passed: %s\n", fname);
		return true;
	} catch (const Failure& f) {
		printf("** FAILED: %s\n", fname);
		fprintf(stderr, "Failure in test %s:\n%s\n", fname, f.msg.c_str());
	} catch (const std::exception& e) {
		printf("** FAILED: %s\n", fname);
		fprintf(stderr, "Exception in test %s:\n%s\n", fname, e.what());
	} catch (...) {
		printf("** FAILED: %s\n", fname);
		fprintf(stderr, "Exception in test %s:\nUnknown exception type\n",
				fname);
	}
	unit_test_failures++;
	unit_test_suite_failures++;
	return false;
}

void unit_test_reset_counts() {
	unit_test_runs = 0;
	unit_test_failures = 0;
}

void unit_test_print_count() {
	fprintf(stdout, "Ran %d tests with %d passes and %d failures\n",
			unit_test_runs, unit_test_runs - unit_test_failures,
			unit_test_failures);
	fflush(stdout);
}

bool unit_test_suite(unit_test_function func, const char* fname) {
	unit_test_suite_runs = 0;
	unit_test_suite_failures = 0;
	fprintf(stdout, "%s\n", fname);
	func();
	if (unit_test_suite_failures > 0) {
		fprintf(stdout,
				"FAILED %s test suite with %d tests failing out of %d]\n",
				fname, unit_test_suite_failures, unit_test_suite_runs);
	} else {
		fprintf(stdout, "%s passed all %d tests\n\n", fname, unit_test_suite_runs, unit_test_suite_runs);
	}
	fflush(stdout);
	return true;
}
