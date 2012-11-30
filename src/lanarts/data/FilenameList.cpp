/*
 * FilenameList.cpp:
 *  Convenient type alias for stl container
 */

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

#include <common/strformat.h>
#include <common/fatal_error.h>

#include "FilenameList.h"

bool filenames_from_pattern(FilenameList & filenames, const std::string& str) {
	size_t pos1 = str.find_first_of("[]");

	if (pos1 == std::string::npos) {
		filenames.push_back(str);
		return false;
	}

	int pos2 = -1, min = -1, max = -1;
	int num_set = sscanf(&str[pos1 + 1], "%d-%d]%n", &min, &max, &pos2);

	if (str[pos1] == ']' || pos2 == -1) {
		printf("Bad file pattern '%s', expected "
				"something like 'filename[0-9].png'\n", str.c_str());
		fatal_error();
	}

	std::string prestring(str.begin(), str.begin() + pos1);
	int post_pos = pos2 + pos1 + 1; // adjust pos2 for whole string

	for (int n = min; n <= max; n++) {
		filenames.push_back(
				format("%s%d%s", prestring.c_str(), n, str.c_str() + post_pos));
	}
	return true;
}
