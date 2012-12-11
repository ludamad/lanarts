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
		fflush(stdout);
		return true;
	} catch (const Failure& f) {
		fprintf(stderr, "Failure in test %s:\n%s\n", fname, f.msg.c_str());
		printf("** FAILED: %s\n", fname);
	} catch (const std::exception& e) {
		printf("** FAILED: %s\n", fname);
		fprintf(stderr, "Exception in test %s:\n%s\n", fname, e.what());
	} catch (...) {
		printf("** FAILED: %s\n", fname);
		fprintf(stderr, "Exception in test %s:\nUnknown exception type\n",
				fname);
	}
	fflush(stdout);
	fflush(stderr);

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
	fprintf(stdout, "running %s\n", fname);
	func();
	if (unit_test_suite_failures > 0) {
		fprintf(stdout, "FAILED %s tests with %d tests failing out of %d\n",
				fname, unit_test_suite_failures, unit_test_suite_runs);
	} else {
		fprintf(stdout, "%s passed all %d tests\n\n", fname,
				unit_test_suite_runs, unit_test_suite_runs);
	}
	fflush(stdout);
	return true;
}


#include <UnitTest++.h>

using namespace UnitTest;

class _UnitTestReporter: public TestReporter {
public:

	_UnitTestReporter() {
		// Unfortunately, there is no 'ReportSuccess'
		// We use 'did_finish_correctly' to track successes
		did_finish_correctly = false;
	}

	virtual ~_UnitTestReporter() {
	}

	virtual void ReportTestStart(const TestDetails& test) {
		did_finish_correctly = true;
	}

	virtual void ReportFailure(const TestDetails& details,
			char const* failure) {

		printf("FAILED: %s line %d (%s)\n", details.testName,
				details.lineNumber, failure);

		did_finish_correctly = false;
	}

	virtual void ReportTestFinish(const TestDetails& details,
			float secondsElapsed) {
		if (did_finish_correctly) {
			printf("Passed: %s of %s\n", details.suiteName, details.testName);
		}
	}

	virtual void ReportSummary(int totalTestCount, int failedTestCount,
			int failureCount, float secondsElapsed) {

		if (failedTestCount > 0) {
			printf("TEST SUITE FAILURE: Not all tests have passed!\n");
		}

		printf("Total tests run: %d\n", totalTestCount);
		printf("Test results: passed: %d; failed: %d\n",
				totalTestCount - failedTestCount, failedTestCount);
	}

private:
	bool did_finish_correctly;
};

int run_unittests() {
	_UnitTestReporter reporter;
	TestRunner runner(reporter);

	return runner.RunTestsIf(Test::GetTestList(), NULL /*All suites*/,
			True() /*All tests*/, 0 /*No time limit*/);
}
