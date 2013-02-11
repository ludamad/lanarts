/*
 * SolidityGridRef.h:
 *
 *  Efficiently represents whether a given square is solid or not.
 *  This shared data is necessary for an efficient, simple format
 *  that pathfinding algorithms can use without copying.
 *
 *  As well it avoids problems with the pathfinding algorithms
 *  being too coupled with the game tile structure, which can change.
 *
 *  This is shared with GameTiles and the various pathfinding structures.
 */

#ifndef SOLIDITYGRIDREF_H_
#define SOLIDITYGRIDREF_H_

#include <lcommon/smartptr.h>
#include "util/Grid.h"

// Until any special implementation is needed, simply define as a typedef
typedef smartptr< Grid<bool> > SolidityGridRef;

#endif /* SOLIDITYGRIDREF_H_ */
