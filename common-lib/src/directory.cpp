/*
 * directory.cpp:
 *  Simple function to create a directory.
 *  Supports Windows and Posix platforms.
 */

#include <string>
#include <cstdlib>
#include "directory.h"
#include "tinydir.h"

#ifdef WIN32

#include "dirent_windows.h"
#include <windows.h>
#include <direct.h>

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
#include <unistd.h>

bool is_directory(const char* path) {
	struct stat st;
	return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

bool create_directory(const char* path) {
	return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}

#endif

bool ensure_directory(const char* path) {
	if (create_directory(path)) {
		return true;
	}
	return is_directory(path);
}

std::string working_directory() {
#ifdef WIN32
	char* cwd = _getcwd(NULL, 0);
#else
	const int CWD_BUFFER_TRY = 1024;
	char buffer[CWD_BUFFER_TRY];
	if (getcwd(buffer, CWD_BUFFER_TRY)) {
		return buffer;
	}
	size_t size = CWD_BUFFER_TRY * 2;
	char* cwd = (char*)malloc(size);
	while (!getcwd(cwd, size)) {
		size *= 2;
		cwd = (char*)realloc(cwd, size);
	}
#endif
	std::string ret(cwd);
	free(cwd);
	return ret;
}

DirListing list_directory(const char* path) {
	DirListing listing;
	return listing;
}

FilenameList search_directory(const char* path, const char* name,
		const char* ext, bool recursive) {
}
