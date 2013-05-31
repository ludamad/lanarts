/*
 * map_fill.cpp:
 *  Various algorithms for filling a map.
 */

#include <cstdlib>

#include "map_fill.h"
#include "bsp.hpp"

namespace ldungeon_gen {

	RectangleApplyOperator::RectangleApplyOperator(
			ConditionalOperator fill_oper, bool create_subgroup) :
					fill_oper(fill_oper),
					perimeter(0),
					perimeter_oper(Selector(0, 0), Operator(0, 0, 0)),
					create_subgroup(create_subgroup) {
	}

	RectangleApplyOperator::RectangleApplyOperator(
			ConditionalOperator fill_oper, int perimeter,
			ConditionalOperator perimeter_oper, bool create_subgroup) :
					fill_oper(fill_oper),
					perimeter(perimeter),
					perimeter_oper(perimeter_oper),
					create_subgroup(create_subgroup) {
	}

	bool RectangleApplyOperator::apply(Map& map, group_t parent_group_id, const BBox& rect) {

		const BBox map_bounds(Pos(0, 0), map.size());

		BBox inner_rect = rect.shrink(perimeter);
		if (create_subgroup) {
			map.make_group(inner_rect, parent_group_id);
		}
		int rowstart = rect.y1 * map.width();
		for (int y = rect.y1; y < rect.y2; y++) {

			int idx = rowstart + rect.x1;
			for (int x = rect.x1; x < rect.x2; x++) {
				if (perimeter == 0 || inner_rect.contains(x, y)) {
					/* Inside the box */
					map.raw_get(idx).apply(fill_oper);
				} else {
					/* On the perimeter*/
					map.raw_get(idx).apply(perimeter_oper);
				}
				idx++;
			}
			rowstart += map.width();
		}
		return true;
	}

	/* By placing in cpp file, ensures only created in one module */
	AreaOperatorBase::~AreaOperatorBase() {
	}

	class BSPRectOperator: public ITCODBspCallback {
	public:
		BSPRectOperator(Map& map, group_t parent_group_id, const BSPApplyOperator& bsp_settings,
				AreaOperatorBase& node_oper) :
						map(map),
						parent_group_id(parent_group_id),
						bsp_settings(bsp_settings),
						node_oper(node_oper) {
		}

		int get_int(int min, int max) {
			if (min > max)
				std::swap(min, max);
			if (min == max)
				return min;
			return bsp_settings.randomizer.rand(min, max);
		}

		virtual bool visitNode(TCODBsp *node, void * __unused) {
			if (node->isLeaf()) {
				// calculate the room size
				int minx = node->x, maxx = node->x + node->w;
				int miny = node->y, maxy = node->y + node->h;
				if (bsp_settings.randomize_size) {
					minx = get_int(minx, maxx - bsp_settings.minimum_room_size.w + 2);
					miny = get_int(miny, maxy - bsp_settings.minimum_room_size.h + 2);
					maxx = get_int(minx + bsp_settings.minimum_room_size.w - 1, maxx + 1);
					maxy = get_int(miny + bsp_settings.minimum_room_size.h - 1, maxy + 1);
				}
				node_oper.apply(map, parent_group_id, BBox(minx, miny, maxx, maxy));
			}
			return true;
		}

	private:
		Map& map;
		group_t parent_group_id;
		const BSPApplyOperator& bsp_settings;
		AreaOperatorBase& node_oper;
	};

	bool BSPApplyOperator::apply(Map& map, group_t parent_group_id, const BBox& rect) {
		if (create_subgroup) {
			map.make_group(rect, parent_group_id);
		}

		TCODBsp* bsp = new TCODBsp(rect.x1, rect.y1, rect.width(),
				rect.height());

		bsp->splitRecursive(randomizer, split_depth, minimum_room_size.w,
				minimum_room_size.h, 1.5f, 1.5f);

		// create the dungeon from the bsp
		BSPRectOperator bsp_node_oper(map, parent_group_id, *this, area_oper);
		bsp->traverseInvertedLevelOrder(&bsp_node_oper, NULL);
		delete bsp;

		return true;
	}
}
