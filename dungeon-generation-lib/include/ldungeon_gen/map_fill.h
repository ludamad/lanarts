/*
 * map_fill.h:
 *  Various algorithms for filling a map.
 *  Namely:
 *  	- Tightly packing an area with rectangales
 *  	- Random rectangle placement
 */

#ifndef MAP_FILL_H_
#define MAP_FILL_H_

#include "Map.h"

namespace ldungeon_gen {

	void rectangle_apply(Map& map, const BBox& bbox,
			ConditionalOperator fill_oper);
	void rectangle_apply(Map& map, const BBox& bbox,
			ConditionalOperator fill_oper, int perimeter,
			ConditionalOperator perimeter_oper);
	void corners_apply(Map& map, const BBox& bbox, ConditionalOperator oper);

	/*
	 * Operates on a BSP leaf node's area
	 */
	class BSPNodeOperator {
	public:
		virtual ~BSPNodeOperator();
		virtual void apply(Map& map, const BBox& rect) = 0;
	};

	/* Performs a 'ConditionalOperator' on a BSP node
	 * Labels each node with a successive group */
	class RectangleOperator: public BSPNodeOperator {
	public:
		RectangleOperator(ConditionalOperator fill_oper, int parent_group =
				ROOT_GROUP_ID);
		RectangleOperator(ConditionalOperator fill_oper, int perimeter,
				ConditionalOperator perimeter_oper, int parent_group =
						ROOT_GROUP_ID);
		virtual ~RectangleOperator();
		virtual void apply(Map& map, const BBox& rect);
	private:
		int parent_group;
		ConditionalOperator fill_oper;
		int perimeter;
		ConditionalOperator perimeter_oper;
	};

	struct BSPSettings {
		bool randomize_size;
		Size minimum_room_size;
		int split_depth;
		BSPSettings(Size minimum_room_size, bool randomize_size = true,
				int split_depth = 8) :
						randomize_size(randomize_size),
						minimum_room_size(minimum_room_size),
						split_depth(split_depth) {
		}
	};

	void bsp_rect_apply(Map& map, const BSPSettings& settings,
			MTwist& randomizer, const BBox& rect, BSPNodeOperator& node_oper);

}

#endif /* MAP_FILL_H_ */
