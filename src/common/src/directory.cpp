/*
 * directory.cpp:
 *  Simple function to create a directory.
 *  Supports Windows and Posix platforms.
 */

#include "directory.h"

#ifdef WIN32

#include <windows.h>

bool is_directory(const char* path) {
	DWORD attribs = GetFileAttributesA(path);
	if (attribs == INVALID_FILE_ATTRIBUTES) {
		return false;
	}
	return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}

bool create_directory(const char* path) {
	return CreateDirectory(path, NULL);
}

#else

#include <sys/stat.h>

bool is_directory(const char* path) {
	struct stat st;
	return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

bool create_directory(const char* path) {
	return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}

#endif

bool ensure_directory(const char* path) {
	create_directory(path);
	return is_directory(path);
}
