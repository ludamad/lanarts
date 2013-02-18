#include <cstdio>
#include <stdexcept>
#include <cassert>

#include "unittest.h"

using namespace UnitTest;

struct Failure: public std::runtime_error {
	Failure(const std::string& msg) :
			runtime_error(msg) {
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

		printf("[%s]\n\t**FAILED: %s line %d (%s)\n", details.suiteName,
				details.testName, details.lineNumber, failure);

		did_finish_correctly = false;
	}

	virtual void ReportTestFinish(const TestDetails& details,
			float secondsElapsed) {
		if (did_finish_correctly) {
			printf("[%s]\n\t%s passed \n", details.suiteName, details.testName);
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
