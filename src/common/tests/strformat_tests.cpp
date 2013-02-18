#include "unittest.h"
#include "strformat.h"

static void vararg_str_format_macro_test_helper(std::string& str,
		const char* fmt, ...) {
	VARARG_STR_FORMAT(str, fmt);
}

SUITE(strformat_tests) {
	TEST(str_pack_test) {
		CHECK("test" == str_pack(" test "));
		CHECK("te st" == str_pack("te st"));
		CHECK("test test" == str_pack(" test \n test"));
		CHECK("A B C" == str_pack(" A     \n     B       \n     C    "));
	}

	TEST(strformat_test) {
		{
			std::string teststr = format("%d%s%d", 1, "Hey", 1);
			CHECK(teststr == "1Hey1");
		}
		{
			std::string long_string;
			long_string.resize(1025, 'a');
			std::string teststr = format("%s", long_string.c_str());
			CHECK(teststr == long_string);
		}
	}

	TEST(inplace_strformat_test) {
		std::string inplacestr, inplacestr2;
		{
			format(inplacestr, "%d%s%d", 1, "Hey", 1);
			CHECK(inplacestr == "1Hey1");

			vararg_str_format_macro_test_helper(inplacestr2, "%d%s%d", 1, "Hey",
					1);
			CHECK(inplacestr2 == "1Hey1");
		}
		{
			std::string long_string;
			long_string.resize(1025, 'a');
			format(inplacestr, "%s", long_string.c_str());
			CHECK(inplacestr == long_string);

			vararg_str_format_macro_test_helper(inplacestr2, "%s",
					long_string.c_str());
			CHECK(inplacestr2 == long_string);
		}
		{
			format(inplacestr, "%d%s%d", 1, "Hey", 1);
			CHECK(inplacestr == "1Hey1");

			vararg_str_format_macro_test_helper(inplacestr2, "%d%s%d", 1, "Hey",
					1);
			CHECK(inplacestr2 == "1Hey1");
		}
	}
}
