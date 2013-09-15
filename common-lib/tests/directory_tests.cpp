#include "directory.h"
#include "unittest.h"

SUITE(directory_tests) {
	TEST(list_directory) {
		DirListing listing = list_directory(".");
		bool has_self = false;
		for (int i = 0; i < listing.size(); i++) {
			if (listing[i].name == "lcommon_tests") {
				has_self = true;
			}
		}
		CHECK(has_self);
	}

	TEST(search_directory) {
		FilenameList results = search_directory(".", "*o*o*_t*");
		bool has_self = false;
		for (int i = 0; i < results.size(); i++) {
			if (results[i] == "./lcommon_tests") {
				has_self = true;
			}
		}
		CHECK(has_self);
	}
}
