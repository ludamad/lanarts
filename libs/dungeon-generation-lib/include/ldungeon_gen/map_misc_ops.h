/*
 * map_misc_ops.h:
 *  Connectivity analysis via floodfill and other operations.
 */

#ifndef LDUNGEON_MISC_OPS_H_
#define LDUNGEON_MISC_OPS_H_

#include "Map.h"

#include <lcommon/smartptr.h>
#include <vector>

namespace ldungeon_gen {
    void erode_diagonal_pairs(Map& map, MTwist& rng, BBox area, Selector candidate);
}

#endif /* LDUNGEON_MISC_OPS_H_ */
