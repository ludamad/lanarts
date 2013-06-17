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
	AreaTemplate::AreaTemplate(const char* data, int data_width,
			Size size) {
		_grid.set(new Grid<char>(size));
		LDUNGEON_ASSERT(size.w > 0 && size.h > 0 && data_width >= size.w);

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

	void AreaTemplate::apply(MapPtr map, group_t parent_group_id,
			const Pos& xy, bool flipX, bool flipY, bool create_subgroup) {
		int w = _grid->width(), h = _grid->height();

		/* Push arguments only once, the moment we find our lua_State */
		lua_State* L = NULL;

		/* Area of map to apply to: */
		if (create_subgroup) {
			BBox rect(xy, _grid->size());
			map->make_group(rect, parent_group_id);
		}
		BBox grid_rect(Pos(), _grid->size());
		FOR_EACH_BBOX(grid_rect, x, y) {
			int sqrx = xy.x + (flipX ? w - x - 1 : x);
			int sqry = xy.y + (flipY ? h - y - 1 : y);
			char chr = (*_grid)[Pos(x,y)];
			Glyph glyph = _legend[chr];

			(*map)[Pos(sqrx, sqry)] = glyph.square;

			/* Apply lua function */
			if (!glyph.value.empty() && !glyph.value.isnil()) {
				LDUNGEON_ASSERT(L == NULL || glyph.value.luastate() == L);
				if (L == NULL) {
					L = glyph.value.luastate();
					luawrap::push(L, map);
					luawrap::push(L, Pos());
				}
				LuaStackValue mapval(L, -2), posval(L, -1);
				glyph.value.push();
				mapval.push();
				posval.push();
				lua_call(L, 2, 0);
			}
		}

		if (L) {
			/* Pop cached map and position objects */
			lua_pop(L, 2);
		}
	}

}
