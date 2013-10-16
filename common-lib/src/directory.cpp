/*
 * directory.cpp:
 *  Simple function to create a directory.
 *  Supports Windows and Posix platforms.
 */

#include "lcommon_assert.h"
#include <string>
#include <cstdlib>
#include <cstdio>
#include "directory.h"
#include "tinydir.h"

#ifdef WIN32

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
	tinydir_dir dir;
	tinydir_open(&dir, path);

	while (dir.has_next) {
		tinydir_file file;
	    tinydir_readfile(&dir, &file);
	    if (strcmp(file.name,".") != 0 && strcmp(file.name,"..") != 0) {
	    	// Avoid dot-files
			listing.push_back(DirEntry(file.is_dir, file.name, file.path));
	    }
	    tinydir_next(&dir);
	}

	tinydir_close(&dir);
	return listing;
}

static bool matches_wildcard(const char* pattern, const char* str) {
	const char* pattern_ptr = pattern, *str_ptr = str;
	while (*pattern) {
		if (*pattern == '*') {
			char avoid = *(++pattern);
			LCOMMON_ASSERT(avoid != '*');
			while (*str_ptr && *str_ptr != avoid) { str_ptr++; }
		} else {
			if (*(pattern++) != *(str_ptr++)) {
				return false;
			}
		}
	}

	// Did we match the whole string ?
	return (*str_ptr == 0);
}


static void search_directory(FilenameList& results, const char* path, const char* name_pattern, bool recursive) {
	DirListing listing = list_directory(path);
	for (int i = 0; i < listing.size(); i++) {
		DirEntry e = listing[i];
		if (e.is_directory && recursive) {
			search_directory(results, e.path.c_str(), name_pattern, recursive);
		}
		if (!e.is_directory && matches_wildcard(name_pattern, e.name.c_str())) {
			results.push_back(path + ("/" + e.name));
		}
	}
}

FilenameList search_directory(const char* path, const char* name_pattern, bool recursive) {
	FilenameList results;
	search_directory(results, path, name_pattern, recursive);
	return results;
}
