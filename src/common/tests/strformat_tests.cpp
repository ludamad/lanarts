#include "../unittest.h"
#include "../strformat.h"

static void vararg_str_format_macro_test(std::string& str, const char* fmt, ...) {
	VARARG_STR_FORMAT(str, fmt);
}

static void strformat_test() {
	{
		std::string teststr = format("%d%s%d", 1, "Hey", 1);
		UNIT_TEST_ASSERT(teststr == "1Hey1");
	}
	{
		std::string long_string;
		long_string.resize(1025, 'a');
		std::string teststr = format("%s", long_string.c_str());
		UNIT_TEST_ASSERT(teststr == long_string);
	}
}

static void inplace_strformat_test() {
	std::string inplacestr, inplacestr2;
	{
		format(inplacestr, "%d%s%d", 1, "Hey", 1);
		UNIT_TEST_ASSERT(inplacestr == "1Hey1");

		vararg_str_format_macro_test(inplacestr2, "%d%s%d", 1, "Hey", 1);
		UNIT_TEST_ASSERT(inplacestr2 ==  "1Hey1");
	}
	{
		std::string long_string;
		long_string.resize(1025, 'a');
		format(inplacestr, "%s", long_string.c_str());
		UNIT_TEST_ASSERT(inplacestr == long_string);

		vararg_str_format_macro_test(inplacestr2, "%s", long_string.c_str());
		UNIT_TEST_ASSERT(inplacestr2 == long_string);
	}
	{
		format(inplacestr, "%d%s%d", 1, "Hey", 1);
		UNIT_TEST_ASSERT(inplacestr == "1Hey1");

		vararg_str_format_macro_test(inplacestr2, "%d%s%d", 1, "Hey", 1);
		UNIT_TEST_ASSERT(inplacestr2 ==  "1Hey1");
	}
}
void strformat_tests() {
	UNIT_TEST(strformat_test);
	UNIT_TEST(inplace_strformat_test);
}
