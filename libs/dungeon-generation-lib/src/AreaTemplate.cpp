/*
 * AreaTemplate.cpp:
 *  Represents a partially defined area. Can be a standalone room template,
 *  or embedded as a small part of another room.
 *  They can be rotated randomly or not.
 */

#include <cstdio>

#include "ldungeon_assert.h"

#include <luawrap/luawrap.h>

#include "AreaTemplate.h"

namespace ldungeon_gen {
	AreaTemplate::AreaTemplate(const char* data, Size size) {
		_grid.set(new Grid<char>(size));
		LDUNGEON_ASSERT(size.w > 0 && size.h > 0);
		/* Assume uniformly spaced block with newline at end of each line */
		int data_width = size.w + 1; /* Account for newline character */

		for (int y = 0; y < size.h; y++) {
			for (int x = 0; x < size.w; x++) {
				int data_index = y * data_width + x;
				(*_grid)[Pos(x,y)] = data[data_index];
			}
		}
	}

	AreaTemplate::~AreaTemplate() {
	}

	void AreaTemplate::define_glyph(char key, Glyph glyph) {
		_legend[key] = glyph;
	}

	inline static Pos reorient(Pos xy, Size size, Orientation orientation) {
		int negx = size.w - xy.x - 1, negy = size.h - xy.y - 1;
		switch (orientation) {
		case ORIENT_DEFAULT:
			return xy;
		case ORIENT_FLIP_X:
			return Pos(negx, xy.y);
		case ORIENT_FLIP_Y:
			return Pos(xy.x, negy);
		case ORIENT_TURN_90:
			return Pos(xy.y, negx);
		case ORIENT_TURN_180:
			return Pos(negx, negy);
		case ORIENT_TURN_270:
			return Pos(negy, xy.x);
		default:
			LDUNGEON_ASSERT(false);
			return xy; // Shutup compiler
		}
	}

	void AreaTemplate::apply(MapPtr map, group_t parent_group_id,
			const Pos& xy, Orientation orientation, bool create_subgroup) {
		int w = _grid->width(), h = _grid->height();

		/* Push arguments only once, the moment we find our lua_State */
		lua_State* L = NULL;

		if (create_subgroup) {
			Size adjustedsize = (orientation == ORIENT_TURN_90 || orientation == ORIENT_TURN_270) ? Size(h,w) : Size(w,h);
			BBox rect(xy, adjustedsize);
			map->make_group(rect, parent_group_id);
		}
                /* Area of map to apply to: */
		BBox grid_rect(Pos(), _grid->size());
		FOR_EACH_BBOX(grid_rect, x, y) {
			Pos sqr = xy + reorient(Pos(x,y), Size(w,h), orientation);
			char chr = (*_grid)[Pos(x,y)];
			Glyph glyph = _legend[chr];

			(*map)[sqr].apply(glyph.oper.oper);

			/* Apply lua function */
			if (!glyph.value.empty() && !glyph.value.isnil()) {
				LDUNGEON_ASSERT(L == NULL || glyph.value.luastate() == L);
				if (L == NULL) {
					L = glyph.value.luastate();
					luawrap::push(L, map);
				}
				LuaStackValue mapval(L, -1);
				glyph.value.push();
				mapval.push();
				luawrap::push(L, sqr);
				lua_call(L, 2, 0);
			}
		}

		if (L) {
			/* Pop cached map and position objects */
			lua_pop(L, 2);
		}
	}

	bool AreaTemplate::matches(MapPtr map, group_t parent_group_id,
			const Pos& xy, Orientation orientation) {
		int w = _grid->width(), h = _grid->height();
                /* Area of map to apply to: */
		BBox grid_rect(Pos(), _grid->size());
		FOR_EACH_BBOX(grid_rect, x, y) {
			Pos sqr = xy + reorient(Pos(x,y), Size(w,h), orientation);
                        if (sqr.x < 0 || sqr.x >= map->width() || sqr.y < 0 || sqr.y >= map->height()) {
                            return false;
                        }
			char chr = (*_grid)[Pos(x,y)];
			Glyph glyph = _legend[chr];

			if (!(*map)[sqr].matches(glyph.oper.selector)) {
                            return false;
                        }
		}
                return true;
	}


}
