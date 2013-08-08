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

	enum Orientation {
		ORIENT_DEFAULT,
		ORIENT_FLIP_X,
		ORIENT_FLIP_Y,
		ORIENT_TURN_90,
		ORIENT_TURN_180,
		ORIENT_TURN_270
	};

	struct Glyph {
		ConditionalOperator oper;
		LuaValue value; /* event, passed map and xy */
		Glyph(ConditionalOperator oper = ConditionalOperator(), LuaValue value = LuaValue()) :
				oper(oper), value(value) {
		}
	};

	/* Attempts to place a template in the given location */
	class AreaTemplate {
	public:
		AreaTemplate(const char* data, Size size);
		~AreaTemplate();

		void define_glyph(char key, Glyph glyph);
		void apply(MapPtr map, group_t parent_group_id, const Pos& xy,
				Orientation orientation, bool create_subgroup = true);

		Size size() const {
			return _grid->size();
		}
	private:
		smartptr< Grid<char> > _grid;
		std::map<char, Glyph> _legend;
	};

	typedef smartptr<AreaTemplate> AreaTemplatePtr;
}
#endif /* LDUNGEON_AREATEMPLATE_H_ */
