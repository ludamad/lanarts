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
    bool area_fully_connected(Map& map, BBox area, Selector unfilled, Operator mark, Selector marked);
    void area_fill_unconnected(Map& map, BBox area, Pos seed, Selector unfilled, Operator mark, Selector marked, Operator fill);

    void erode_diagonal_pairs(Map& map, MTwist& rng, BBox area, Selector candidate);
}

#endif /* LDUNGEON_MISC_OPS_H_ */
