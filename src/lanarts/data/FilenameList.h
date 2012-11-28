/*
 * FilenameList.h:
 *  Functions for lists of files, and a convenient type alias for an stl container
 */

#ifndef FILENAMELIST_H_
#define FILENAMELIST_H_

#include <vector>
#include <string>

typedef std::vector<std::string> FilenameList;

void filenames_from_pattern(FilenameList & filenames, const std::string& str);

#endif /* FILENAMELIST_H_ */
