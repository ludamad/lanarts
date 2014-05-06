/*
 * typename.h:
 *  Utility for getting the (clean) name of a type
 */

#ifndef TYPENAME_H_
#define TYPENAME_H_

#include <string>
#include <typeinfo>

std::string cpp_demangle_type_name(const char* name);

template <typename T>
inline std::string cpp_type_name(const T& object) {
	return cpp_demangle_type_name(typeid(object).name());
}

#endif
