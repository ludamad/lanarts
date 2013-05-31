/*
 * map_fill.h:
 *  Various algorithms for filling a map.
 *  Namely:
 *  	- Tightly packing an area with rectangales
 *  	- Random rectangle placement
 */

#ifndef MAP_FILL_H_
#define MAP_FILL_H_

#include <lcommon/Range.h>

#include "Map.h"

namespace ldungeon_gen {

	/* Abstract operation on an area of a map */
	class AreaOperatorBase {
	public:
		virtual ~AreaOperatorBase();
		/* Should only return false if _nothing_ was done! */
		virtual bool apply(Map& map, group_t parent_group_id, const BBox& rect) = 0;
	};

	/* Applies an operator to a rectangle, can operate separately on the perimeter */
	struct RectangleApplyOperator: public AreaOperatorBase {
		ConditionalOperator fill_oper, perimeter_oper;
		int perimeter;
		bool create_subgroup;

		/* Apply to rectangle without a perimeter */
		RectangleApplyOperator(ConditionalOperator fill_oper, bool create_subgroup = true);

		/* Apply to rectangle with a perimeter */
		RectangleApplyOperator(ConditionalOperator fill_oper, int perimeter,
				ConditionalOperator perimeter_oper, bool create_subgroup = true);

		virtual bool apply(Map& map, group_t parent_group_id, const BBox& rect);
	};

	struct BSPApplyOperator: public AreaOperatorBase {
		MTwist& randomizer;
		bool randomize_size;
		Size minimum_room_size;
		int split_depth;
		bool create_subgroup;

		AreaOperatorBase& area_oper;

		BSPApplyOperator(MTwist& randomizer, AreaOperatorBase& area_oper,
				Size minimum_room_size, bool randomize_size = true,
				int split_depth = 8, bool create_subgroup = true) :
						area_oper(area_oper),
						randomizer(randomizer),
						randomize_size(randomize_size),
						minimum_room_size(minimum_room_size),
						split_depth(split_depth),
						create_subgroup(create_subgroup) {
		}

		virtual bool apply(Map& map, group_t parent_group_id, const BBox& rect);
	};

	struct RandomPlacementApplyOperator : public AreaOperatorBase {
		int region_padding;
		Range amount_of_regions, size;
		bool create_subgroup;

		AreaOperatorBase& area_oper;

		RandomPlacementApplyOperator(int region_padding, Range amount_of_regions, Range size,
				AreaOperatorBase& area_oper, bool create_subgroup = true) :
						region_padding(region_padding),
						amount_of_regions(amount_of_regions),
						size(size),
						area_oper(area_oper),
						create_subgroup(create_subgroup) {
		}
	};
}

#endif /* MAP_FILL_H_ */
