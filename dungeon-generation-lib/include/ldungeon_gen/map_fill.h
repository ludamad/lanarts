/*
 * map_fill.h:
 *  Various algorithms for filling a map.
 *  Namely:
 *  	- Tightly packing an area with rectangales
 *  	- Random rectangle placement
 */

#ifndef LDUNGEON_MAP_FILL_H_
#define LDUNGEON_MAP_FILL_H_

#include <lcommon/Range.h>
#include <lcommon/smartptr.h>

#include "Map.h"
#include "map_check.h"

namespace ldungeon_gen {

	/* Abstract operation on an area of a map */
	class AreaOperatorBase {
	public:
		virtual ~AreaOperatorBase();
		/* Should only return false if _nothing_ was done! */
		virtual bool apply(MapPtr map, group_t parent_group_id, const BBox& rect) = 0;
	};

	typedef smartptr<AreaOperatorBase> AreaOperatorPtr;

	/* Applies an operator to a rectangle, can operate separately on the perimeter */
	struct RectangleApplyOperator: public AreaOperatorBase {
		AreaQueryPtr query;
		ConditionalOperator fill_oper, perimeter_oper;
		int perimeter;
		bool create_subgroup;

		/* Apply to rectangle without a perimeter */
		RectangleApplyOperator(AreaQueryPtr query, ConditionalOperator fill_oper, bool create_subgroup = true);

		/* Apply to rectangle with a perimeter */
		RectangleApplyOperator(AreaQueryPtr query, ConditionalOperator fill_oper, int perimeter,
				ConditionalOperator perimeter_oper, bool create_subgroup = true);

		virtual bool apply(MapPtr map, group_t parent_group_id, const BBox& rect);
	};

	struct BSPApplyOperator: public AreaOperatorBase {
		MTwist& randomizer;
		bool randomize_size;
		Size minimum_room_size;
		int split_depth;
		bool create_subgroup;

		AreaOperatorPtr area_oper;

		BSPApplyOperator(MTwist& randomizer, AreaOperatorPtr area_oper,
				Size minimum_room_size, bool randomize_size = true,
				int split_depth = 8, bool create_subgroup = true) :
						area_oper(area_oper),
						randomizer(randomizer),
						randomize_size(randomize_size),
						minimum_room_size(minimum_room_size),
						split_depth(split_depth),
						create_subgroup(create_subgroup) {
		}

		virtual bool apply(MapPtr map, group_t parent_group_id, const BBox& rect);
	};

	struct RandomPlacementApplyOperator : public AreaOperatorBase {
        MTwist& randomizer;
		Range amount_of_regions, size;
		bool create_subgroup;

		AreaOperatorPtr area_oper;

		RandomPlacementApplyOperator(MTwist& randomizer,
                Range amount_of_regions, Range size,
				AreaOperatorPtr area_oper, bool create_subgroup = true) :
                        randomizer(randomizer),
						amount_of_regions(amount_of_regions),
						size(size),
						area_oper(area_oper),
						create_subgroup(create_subgroup) {
		}

		virtual bool apply(MapPtr map, group_t parent_group_id, const BBox& rect);

    private:
        bool place_random(MapPtr map, group_t parent_group_id, Size size);
	};
}

#endif /* LDUNGEON_MAP_FILL_H_ */
