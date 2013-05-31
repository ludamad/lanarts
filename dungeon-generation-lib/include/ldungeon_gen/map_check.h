/*
 * map_check.h:
 *  Various algorithms for querying a map.
 */

#ifndef MAP_CHECK_H_
#define MAP_CHECK_H_

#include "Map.h"

namespace ldungeon_gen {

	/* Query if an entire rectangle matches the given selector */
	bool rectangle_matches(Map& map, const BBox& bbox,
			Selector selector);
	/* Query if the inside of a rectangle matches a certain selector,
	 * and if the perimeter of the rectangle matches a different selector */
	bool rectangle_matches(Map& map, const BBox& bbox,
			Selector fill_selector, int perimeter, Selector perimeter_selector);
}

#endif /* MAP_CHECK_H_ */
