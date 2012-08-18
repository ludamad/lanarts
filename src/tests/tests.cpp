#include "tests.h"
#include <cstdio>
#include <stdexcept>

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

bool unit_test(unit_test_function func, const char* fname) {
	unit_test_runs++;
	try {
		func();
		printf("Test %s passed.\n", fname);
		return true;
	} catch (const Failure& f) {
		fprintf(stderr, "Failure in test %s:\n\t%s\n", fname, f.msg.c_str());
	} catch (const std::exception& e) {
		fprintf(stderr, "Exception in test %s:\n\t%s\n", fname, e.what());
	} catch (...) {
		fprintf(stderr, "Exception in test %s:\n\tUnknown exception type\n",
				fname);
	}
	unit_test_failures++;
	return false;
}

void lua_unit_tests();
void net_unit_tests();
void stat_serialize_tests();
void adv_net_unit_tests();

void run_unit_tests() {
	lua_unit_tests();
	net_unit_tests();
	stat_serialize_tests();
	adv_net_unit_tests();
	fprintf(stdout, "Ran %d tests with %d passes and %d failures\n",
			unit_test_runs, unit_test_runs - unit_test_failures,
			unit_test_failures);
	fflush(stdout);
}

