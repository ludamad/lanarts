/*
 * FilenameList.h:
 *  Functions for lists of files, and a convenient type alias for an stl container
 */

#ifndef FILENAMELIST_H_
#define FILENAMELIST_H_

#include <vector>
#include <string>

typedef std::vector<std::string> FilenameList;

/** Expands a pattern of the form 'file(0-9).png' into a list of filenames.
 *  If the pattern syntax (square brackets) are not found, false is returned,
 *  and only the string is pushed.
 */
bool filenames_from_pattern(FilenameList& filenames, const std::string& str);

#endif /* FILENAMELIST_H_ */
