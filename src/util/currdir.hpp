#pragma once

#ifdef _WIN32
#include "direct.h"
#define PATH_SEP '\\'
#define chdir _chdir
#else
#include "unistd.h"
#define PATH_SEP '/'
#endif

#include <cstring>
#include <string>
#include <iostream>

inline std::string get_basename(const std::string& path) {
    auto path_directory_index = path.find_last_of(PATH_SEP);
    return path.substr(0 , path_directory_index + 1);
}

inline bool chdir(const std::string& dir) {
    return chdir(dir.c_str()) == 0;
}
