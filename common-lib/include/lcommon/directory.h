/*
 * directory.h:
 *  Simple directory functions.
 *  Supports Windows and Posix platforms.
 */

#ifndef DIRECTORY_H_
#define DIRECTORY_H_

#include <string>

/* Returns false if invalid path or not a directory */
bool is_directory(const char* path);

/* Returns false if could not create directory */
bool create_directory(const char* path);

/* Tries to create a directory if one doesn't exist
 * Returns false if cannot (eg same-name file exists) */
bool ensure_directory(const char* path);

/* Grab current working directory. Robust & portable but slightly inefficient. */
std::string working_directory();

#endif /* DIRECTORY_H_ */
