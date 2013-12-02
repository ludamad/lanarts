/*
 * BoolGridRef.h:
 *
 *  Efficiently represents whether a boolean value for a square (1 bit/square).
 *  Enables reference sharing, which is necessary for an efficient, simple format
 *  that pathfinding algorithms can use without copying.
 *
 *  As well it avoids problems with the pathfinding algorithms
 *  being too coupled with the game tile structure, which can change.
 *
 *  This is shared with GameTiles and the various pathfinding structures.
 */

#ifndef BOOLGRIDREF_H_
#define BOOLGRIDREF_H_

#include <lcommon/smartptr.h>
#include <lcommon/Grid.h>

// Until any special implementation is needed, simply define as a typedef
typedef smartptr< Grid<bool> > BoolGridRef;

#endif /* BOOLGRIDREF_H_ */
