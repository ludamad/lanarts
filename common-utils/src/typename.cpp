/*
 * typename.cpp:
 *  Utility for getting the (clean) name of a type
 */

#include "typename.h"

#ifdef __GNUC__
#include <cxxabi.h>

/* Attempt to make the output of typeinfo readable for nicer errors, if possible. */
std::string cpp_demangle_type_name(const char* name) {
	int status;
	const char* demangled = abi::__cxa_demangle(name, 0, 0, &status);
	if (status != 0) {
		return name;
	}
	std::string newname(demangled);
	return newname;
}

#else

/* Attempt to make the output of typeinfo readable for nicer errors, if possible. */
std::string cpp_demangle_type_name(const char* name) {
	return name;
}
#endif
