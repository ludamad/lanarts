#include <cstdio>

#include <common/unittest.h>
#include <common/fatal_error.h>

#include "../src/data/FilenameList.h"

void filenames_from_pattern_test() {
	{
		FilenameList testfiles, expectedfiles;
		filenames_from_pattern(testfiles, "test");
		expectedfiles.push_back("test");
		UNIT_TEST_ASSERT(testfiles == expectedfiles);
	}
	{
		FilenameList testfiles, expectedfiles;
		filenames_from_pattern(testfiles, "test(0-2)");
		expectedfiles.push_back("test0");
		expectedfiles.push_back("test1");
		expectedfiles.push_back("test2");
		UNIT_TEST_ASSERT(testfiles == expectedfiles);
	}
	{
		FilenameList testfiles, expectedfiles;
		filenames_from_pattern(testfiles, "test(0-2).png");
		expectedfiles.push_back("test0.png");
		expectedfiles.push_back("test1.png");
		expectedfiles.push_back("test2.png");
		UNIT_TEST_ASSERT(testfiles == expectedfiles);
	}

	{
		FilenameList testfiles, expectedfiles;
		filenames_from_pattern(testfiles, "test(2-2)");
		expectedfiles.push_back("test2");
		UNIT_TEST_ASSERT(testfiles == expectedfiles);
	}

	{
		bool failed = false;
		try {
			FilenameList testfiles;
			printf("NB: Next failure expected ...\n");
			filenames_from_pattern(testfiles, "test(2-2");
		} catch (const __FatalError& _) {
			failed = true;
		}
		unit_test_assert("Only start bracket should be invalid", failed);
	}

	{
		bool failed = false;
		try {
			FilenameList testfiles;
			printf("NB: Next failure expected ...\n");
			filenames_from_pattern(testfiles, "test2-2)");
		} catch (const __FatalError& _) {
			failed = true;
		}
		unit_test_assert("Only end bracket should be invalid", failed);
	}

}

void parse_unit_tests() {
	UNIT_TEST(filenames_from_pattern_test);
}
