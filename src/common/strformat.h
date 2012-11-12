/*
 * strformat.h:
 *  sprintf-like function for std::string.
 *  Not efficient for very large (>512 char) strings.
 */

#ifndef STRFORMAT_H_
#define STRFORMAT_H_

std::string format(const char* fmt, ...);

#endif /* STRFORMAT_H_ */
