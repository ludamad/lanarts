/*
 * strformat.h:
 *  sprintf-like function for std::string.
 *  Not efficient for very large (>512 char) strings.
 */

#ifndef STRFORMAT_H_
#define STRFORMAT_H_

#include <cstdarg>
#include <string>


std::string str_pack(std::string str);
std::string str_tolower(std::string str);

std::string format(const char* fmt, ...);
void format(std::string& str, const char* fmt, ...);

// Note that because va_copy is not portable, this must be able to fail.
// It is thus 'part' of the solution. The resulting string should not be used if it fails.
// Correct usage:
//   while (1) { va_start(..); if (vformat(..)) break; va_end(..); }
// You can use VARARG_STR_FORMAT to encapsulate this
bool partial_vformat(std::string& str, const char* fmt, va_list ap);

//Macro to encapsulate vformat-like use cases
//Use ONLY where you can use va_start (eg when passed ...)
#define VARARG_STR_FORMAT(str, fmt) \
	{ \
	bool done = false; \
	while (!done) { \
		va_list ap; \
		va_start(ap, fmt); \
		done = partial_vformat(str, fmt, ap); \
		va_end(ap); \
	} \
	}

#endif /* STRFORMAT_H_ */
