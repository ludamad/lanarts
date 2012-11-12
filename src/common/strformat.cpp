/*
 * strformat.cpp:
 *  sprintf-like function for std::string.
 *  Not efficient for very large (>512 char) strings.
 */

#include "strformat.h"

#include <cstdarg>

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
	int size = INIT_SIZE * 2;
	std::string str(size);
	while (1) {
		str.resize(size);
		va_list ap;
		va_start(ap, fmt);
		int n = vsnprintf(&str[0], size, fmt, ap);
		va_end(ap);
		if (n > -1 && n < size) {
			str.resize(n);
			return str;
		}
		size *= 2;
	}
	return str;
}

