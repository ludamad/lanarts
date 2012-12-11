#include <cstdio>

#include <stdexcept>

#include <UnitTest++.h>

void unit_test_assert(const std::string& msg, bool condition) {
	if (!condition) {
		throw std::runtime_error(msg);
	}
}

int main() {
	return UnitTest::RunAllTests();
}
