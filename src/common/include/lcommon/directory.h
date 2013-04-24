/*
 * directory.h:
 *  Simple directory functions.
 *  Supports Windows and Posix platforms
 */

#ifndef DIRECTORY_H_
#define DIRECTORY_H_

/* Returns false if invalid path or not a directory */
bool is_directory(const char* path);

/* Returns false if could not create directory */
bool create_directory(const char* path);

#endif /* DIRECTORY_H_ */
