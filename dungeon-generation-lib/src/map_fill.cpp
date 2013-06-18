/*
 * map_fill.cpp:
 *  Various algorithms for filling a map.
 */

#include <cstdlib>

#include "ldungeon_assert.h"

#include "map_fill.h"
#include "bsp.hpp"

namespace ldungeon_gen {

	RectangleApplyOperator::RectangleApplyOperator(
			AreaQueryPtr query,
			ConditionalOperator fill_oper, bool create_subgroup) :
					query(query),
					fill_oper(fill_oper),
					perimeter(0),
					perimeter_oper(Selector(0, 0), Operator(0, 0, 0)),
					create_subgroup(create_subgroup) {
	}

	RectangleApplyOperator::RectangleApplyOperator(
			AreaQueryPtr query,
			ConditionalOperator fill_oper, int perimeter,
			ConditionalOperator perimeter_oper, bool create_subgroup) :
					query(query),
					fill_oper(fill_oper),
					perimeter(perimeter),
					perimeter_oper(perimeter_oper),
					create_subgroup(create_subgroup) {
	}

	bool RectangleApplyOperator::apply(MapPtr map, group_t parent_group_id, const BBox& rect) {

		const BBox map_bounds(Pos(0, 0), map->size());
		LDUNGEON_ASSERT(rect.resized_within(map_bounds) == rect);

		/* Do we match the query ? */
		if (!query.empty() && !query->matches(map, parent_group_id, rect)) {
			return false;
		}

		BBox inner_rect = rect.shrink(perimeter);
		if (create_subgroup) {
			group_t grp = map->make_group(rect, parent_group_id);
			map->groups.at(grp).group_area = inner_rect;
		}

		int rowstart = rect.y1 * map->width();
		for (int y = rect.y1; y < rect.y2; y++) {

			int idx = rowstart + rect.x1;
			for (int x = rect.x1; x < rect.x2; x++) {
				if (perimeter == 0 || inner_rect.contains(x, y)) {
					/* Inside the box */
					map->raw_get(idx).apply(fill_oper);
				} else {
					/* On the perimeter*/
					map->raw_get(idx).apply(perimeter_oper);
				}
				idx++;
			}
			rowstart += map->width();
		}
		return true;
	}

	/* By placing in cpp file, ensures only created in one module */
	AreaOperatorBase::~AreaOperatorBase() {
	}

	class BSPRectOperator: public ITCODBspCallback {
	public:
		BSPRectOperator(MapPtr map, group_t parent_group_id, const BSPApplyOperator& bsp_settings):
						map(map),
						parent_group_id(parent_group_id),
						bsp_settings(bsp_settings) {
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
					if (minx > maxx || miny > maxy) {
						minx = miny = maxx = maxy = 0;
					}
				}
				bsp_settings.area_oper->apply(map, parent_group_id, BBox(minx, miny, maxx, maxy));
			}
			return true;
		}

	private:
		MapPtr map;
		group_t parent_group_id;
		const BSPApplyOperator& bsp_settings;
	};

	bool BSPApplyOperator::apply(MapPtr map, group_t parent_group_id, const BBox& rect) {
		if (create_subgroup) {
		    parent_group_id = map->make_group(rect, parent_group_id);
		}

		TCODBsp* bsp = new TCODBsp(rect.x1, rect.y1, rect.width(),
				rect.height());

		bsp->splitRecursive(randomizer, split_depth, minimum_room_size.w,
				minimum_room_size.h, 1.5f, 1.5f);

		// create the dungeon from the bsp
		BSPRectOperator bsp_node_oper(map, parent_group_id, *this);
		bsp->traverseInvertedLevelOrder(&bsp_node_oper, NULL);
		delete bsp;

		return true;
	}
    bool RandomPlacementApplyOperator::place_random(MapPtr map, group_t parent_group_id, Size size) {
        const int MAX_ATTEMPTS = 10;
        for (int attempts = 0; attempts < MAX_ATTEMPTS; attempts++) {
            int rx = randomizer.rand(1, map->width() - size.w) | 1;
            int ry = randomizer.rand(1, map->height() - size.h) | 1;
            BBox room(rx, ry, rx + size.w, ry + size.h);
            if (area_oper->apply(map, parent_group_id, room)) {
                return true;
            }
        }
        return false;
    }

    bool RandomPlacementApplyOperator::apply(MapPtr map, group_t parent_group_id, const BBox& rect) {
        const int TOO_MANY_FAILURES = 25;
		if (create_subgroup) {
			parent_group_id = map->make_group(rect, parent_group_id);
		}

        int nrooms = randomizer.rand(amount_of_regions);
        Range sizerange(size.min, size.max);

        int failures = 0;
        for (int i = 0; i < nrooms; i++) {
            for (;; failures++) {

                int rw = randomizer.rand(sizerange), rh = randomizer.rand(sizerange);

                if (place_random(map, parent_group_id, Size(rw, rh))) {
                    break;
                }
                if (failures > TOO_MANY_FAILURES)
                    goto NoMoreRooms;
                // Goto below
            }
        }
        NoMoreRooms:
        return true;
    }
}
