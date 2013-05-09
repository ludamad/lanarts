/*
 * luawrapassert.h:
 *  Assert function used by the luawrap library.
 *  Disable with NDEBUG.
 */

#ifndef LUAWRAPASSERT_H_
#define LUAWRAPASSERT_H_

#include <cassert>

#define LUAWRAP_ASSERT(x) assert(x)

#endif /* LUAWRAPASSERT_H_ */
