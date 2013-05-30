/*
 * map_fill.cpp:
 *  Various algorithms for filling a map.
 */

#include <cstdlib>

#include "map_fill.h"
#include "bsp.hpp"

namespace ldungeon_gen {
	RectangleOperator::RectangleOperator(ConditionalOperator fill_oper,
			int perimeter, ConditionalOperator perimeter_oper, int parent_group) :
					parent_group(parent_group),
					fill_oper(fill_oper),
					perimeter(perimeter),
					perimeter_oper(perimeter_oper) {
	}

	RectangleOperator::RectangleOperator(ConditionalOperator fill_oper,
			int parent_group) :
					parent_group(parent_group),
					fill_oper(fill_oper),
					perimeter(0),
					perimeter_oper(Selector(0, 0), Operator(0, 0, 0)) {
	}

	void RectangleOperator::apply(Map& map, const BBox& rect) {
		BBox inner_rect = rect.shrink(perimeter);
		Group& new_group = map.make_group(inner_rect, parent_group);

		fill_oper.oper.group_value = new_group.group_id;
		fill_oper.oper.use_group_value = true;

		perimeter_oper.oper.group_value = new_group.group_id;
		perimeter_oper.oper.use_group_value = true;

		rectangle_apply(map, inner_rect, fill_oper, perimeter,
				perimeter_oper);
	}
	RectangleOperator::~RectangleOperator() {
	}

	void rectangle_apply(Map& map, const BBox& bbox,
			ConditionalOperator fill_oper) {
		rectangle_apply(map, bbox, fill_oper, 0,
				ConditionalOperator(Selector(0, 0), Operator(0, 0, 0)));
	}

	void rectangle_apply(Map& map, const BBox& bbox,
			ConditionalOperator fill_oper, int perimeter,
			ConditionalOperator perimeter_oper) {

		const BBox map_bounds(Pos(0, 0), map.size());
		BBox perm_bbox = bbox.grow(perimeter).resized_within(map_bounds);

		int rowstart = perm_bbox.y1 * map.width();
		for (int y = perm_bbox.y1; y < perm_bbox.y2; y++) {

			int idx = rowstart + perm_bbox.x1;
			for (int x = perm_bbox.x1; x < perm_bbox.x2; x++) {
				if (bbox.contains(x, y)) {
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
	}

	/* By placing in cpp file, ensures only created in one module */
	BSPNodeOperator::~BSPNodeOperator() {
	}

	static int get_int(MTwist& randomizer, int min, int max) {
		if (min > max)
			std::swap(min, max);
		if (min == max)
			return min;
		return randomizer.rand(min, max);
	}

	class BSPRectOperator: public ITCODBspCallback {
	public:
		BSPRectOperator(Map& map, const BSPSettings& settings, MTwist& mt,
				BSPNodeOperator& node_oper) :
						map(map),
						settings(settings),
						randomizer(mt),
						node_oper(node_oper) {
		}

		virtual bool visitNode(TCODBsp *node, void * __unused) {
			if (node->isLeaf()) {
				// calculate the room size
				int minx = node->x, maxx = node->x + node->w;
				int miny = node->y, maxy = node->y + node->h;
				if (settings.randomize_size) {
					minx = get_int(randomizer, minx,
							maxx - settings.minimum_room_size.w + 2);
					miny = get_int(randomizer, miny,
							maxy - settings.minimum_room_size.h + 2);
					maxx = get_int(randomizer,
							minx + settings.minimum_room_size.w - 1, maxx + 1);
					maxy = get_int(randomizer,
							miny + settings.minimum_room_size.h - 1, maxy + 1);
				}
				node_oper.apply(map, BBox(minx, miny, maxx, maxy));
			}
			return true;
		}

	private:
		Map& map;
		const BSPSettings& settings;
		MTwist& randomizer;
		BSPNodeOperator& node_oper;
	};

	void bsp_rect_apply(Map& map, const BSPSettings& settings,
			MTwist& randomizer, const BBox& rect, BSPNodeOperator& node_oper) {
		TCODBsp* bsp = new TCODBsp(rect.x1, rect.y1, rect.width(),
				rect.height());

		bsp->splitRecursive(randomizer, settings.split_depth,
				settings.minimum_room_size.w, settings.minimum_room_size.h,
				1.5f, 1.5f);

		// create the dungeon from the bsp
		BSPRectOperator bsp_node_oper(map, settings, randomizer, node_oper);
		bsp->traverseInvertedLevelOrder(&bsp_node_oper, NULL);
		delete bsp;
	}
}
