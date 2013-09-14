/*
 * FilenameList.h:
 *  Functions for lists of files, and a convenient type alias for an stl container
 */

#ifndef FILENAMELIST_H_
#define FILENAMELIST_H_

#include <vector>
#include <string>

typedef std::vector<std::string> FilenameList;

/*  Expands a pattern of the form 'file(0-9).png' into a list of filenames.
 *  If the pattern syntax (square brackets) is not found, false is returned,
 *  and the string is treated as a single file name.
 *
 *  Errors on malformed patterns.
 */
bool filenames_from_pattern(FilenameList& filenames, const std::string& str);

#endif /* FILENAMELIST_H_ */
