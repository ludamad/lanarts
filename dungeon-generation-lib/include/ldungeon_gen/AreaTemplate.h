/*
 * AreaTemplate.h:
 *  Represents a partially defined area. Can be a standalone level template,
 *  or embedded as a small part of another level.
 *  They can be rotated randomly or not.
 */

#ifndef LDUNGEON_AREATEMPLATE_H_
#define LDUNGEON_AREATEMPLATE_H_

#include <cstring>
#include <map>

#include <lcommon/mtwist.h>
#include <lcommon/Grid.h>
#include <lcommon/geometry.h>
#include <lcommon/smartptr.h>

#include <luawrap/LuaValue.h>

#include "Map.h"

namespace ldungeon_gen {

	struct Glyph {
		Square square;
		LuaValue value; /* event, passed map and xy */
		Glyph(Square square = Square(), LuaValue value = LuaValue()) : square(square), value(value) {
		}
	};

	/* Attempts to place a template in the given location */
	class AreaTemplate {
	public:
		// TODO TEST
		AreaTemplate(const char* data, int data_width, Size size);
		~AreaTemplate();

		void define_glyph(char key, Glyph glyph);
		void apply(MapPtr map, group_t parent_group_id, const Pos& xy,
				bool flipX = false, bool flipY = false, bool create_subgroup = true);
	private:
		smartptr< Grid<char> > _grid;
		std::map<char, Glyph> _legend;
	};

	typedef smartptr<AreaTemplate> AreaTemplatePtr;
}

#endif /* LDUNGEON_AREATEMPLATE_H_ */
