/*
 * map_check.h:
 *  Various algorithms for querying a map.
 */

#ifndef LDUNGEON_MAP_CHECK_H_
#define LDUNGEON_MAP_CHECK_H_

#include "Map.h"
#include <lcommon/smartptr.h>

namespace ldungeon_gen {

	/* Query if an entire rectangle matches the given selector */
	bool rectangle_matches(Map& map, const BBox& bbox,
			Selector selector);
	/* Query if the inside of a rectangle matches a certain selector,
	 * and if the perimeter of the rectangle matches a different selector */
	bool rectangle_matches(Map& map, const BBox& bbox,
			Selector fill_selector, int perimeter, Selector perimeter_selector);

	/* Abstract operation on an area of a map */
	class AreaQueryBase {
	public:
		virtual ~AreaQueryBase();
		/* Should only return false if _nothing_ was done! */
		virtual bool matches(MapPtr map, group_t parent_group_id, const BBox& rect) = 0;
	};

	typedef smartptr<AreaQueryBase> AreaQueryPtr;

	struct RectangleQuery : public AreaQueryBase {
		Selector fill_selector, perimeter_selector;
		int perimeter;

		RectangleQuery(Selector fill_selector);
		RectangleQuery(Selector fill_selector, int perimeter, Selector perimeter_selector);

		/* Should only return false if _nothing_ was done! */
		virtual bool matches(MapPtr map, group_t parent_group_id, const BBox& rect);
	};

	const int RANDOM_MATCH_MAX_ATTEMPTS = 50;
	bool find_random_square(MTwist& randomizer, MapPtr map, const BBox& bbox, Selector selector, Pos& xy, int max_attempts = RANDOM_MATCH_MAX_ATTEMPTS);
}

#endif /* LDUNGEON_MAP_CHECK_H_ */
