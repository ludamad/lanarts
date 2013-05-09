/*
 * strformat.cpp:
 *  sprintf-like function for std::string.
 *  Not efficient for very large (>512 char) strings.
 */

#include <cstdio>
#include <cstdarg>

#include <algorithm>

#include "strformat.h"

std::string str_pack(std::string str) {
	int slen = str.size();
	int new_i = 0, i = 0;

	do {
		/* First consume spaces at start */
		while (i < slen) {
			if (!isspace(str[i])) {
				break;
			}
			i++;
		}

		/* Copy middle */
		while (i < slen ) {
			if (str[i] == '\n') {
				break;
			}
			str[new_i++] = str[i++];
		}

		/* Trim spaces at end */
		while (new_i >= 0) {
			if (!isspace(str[new_i-1])) {
				break;
			}
			new_i--;
		}

		/* Ensure one space between lines */
		if (i < slen) {
			str[new_i++] = ' ';
		}
	} while (i < slen);

	str.resize(new_i);
	return str; /* return the table */
}

std::string str_tolower(std::string str) {
	for (int i = 0; i < str.size(); i++) {
		str[i] = tolower(str[i]);
	}
	return str;
}

// Note that because va_copy is not portable, this must be able to fail.
// It is thus 'part' of the solution. The resulting string should not be used if it fails.
// Correct usage:
//   while (1) { va_start(..); if (vformat(..)) break; va_end(..); }
// You can use VARARG_STR_FORMAT to encapsulate this
bool partial_vformat(std::string & str, const char *fmt, va_list ap) {
	int size = std::max((int)str.capacity(), 256);
	str.resize(size);
	int n = vsnprintf(&str[0], size, fmt, ap);
	if (n > -1 && n < size) {
		str.resize(n);
		return true;
	}
	str.resize(size * 2);
	return false;
}

void format(std::string & str, const char *fmt, ...) {
	bool done = false;
	while (!done) {
		va_list ap;
		va_start(ap, fmt);
		done = partial_vformat(str, fmt, ap);
		va_end(ap);
	}
}

std::string format(const char *fmt, ...) {
	const int INIT_SIZE = 512;
	char initial_buffer[INIT_SIZE];
	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(initial_buffer, INIT_SIZE, fmt, ap);
	va_end(ap);
	if (n > -1 && n < INIT_SIZE) {
		return std::string(initial_buffer);
	}

	//Dynamic fallback code, note that this ensures correctness but can be very slow.
	std::string str;

	bool done = false;
	while (!done) {
		va_list ap;
		va_start(ap, fmt);
		done = partial_vformat(str, fmt, ap);
		va_end(ap);
	}
	return str;
}
