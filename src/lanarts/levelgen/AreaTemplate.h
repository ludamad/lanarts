/*
 * AreaTemplate.h:
 *  Represents a partially defined area. Can be a standalone level template,
 *  or embedded as a small part of another level.
 *  They can be rotated randomly or not.
 */

#ifndef AREATEMPLATE_H_
#define AREATEMPLATE_H_

#include <cstring>
#include <string>
#include <vector>

#include <common/mtwist.h>
#include <common/geometry.h>

#include "../lanarts_defines.h"
#include "../stats/items/items.h"

class GeneratedLevel;

struct MonsterGenCoord {
	Pos pos;
	enemy_id enemy;
	MonsterGenCoord(Pos pos, enemy_id enemy) :
			pos(pos), enemy(enemy) {
	}
};

struct Glyph {
	char glyph;
	enemy_id enemy;
	Item item;
	Glyph(char glyph, enemy_id enemy, const Item& item = Item()) :
			glyph(glyph), enemy(enemy), item(item) {
	}
};

class AreaTemplate {
public:
	enum {
		EMPTY_GLYPH, WALL_GLYPH, STAIRDOWN_GLYPH, STAIRUP_GLYPH, DOOR_GLYPH
	};
	AreaTemplate(const std::string& name, const char* data, int data_width,
			int width, int height, const std::vector<Glyph>& glyphs);
	~AreaTemplate();

	char* data() {
		return &_data[0];
	}
	int width() const {
		return _width;
	}
	int height() const {
		return _height;
	}
	const std::string& name() {
		return _name;
	}

private:
	std::string _name;
	std::vector<char> _data;
	int _width, _height;
	std::vector<MonsterGenCoord> _monsters;
};

void generate_area(GeneratedLevel& level, AreaTemplate& area_template,
		const Pos& offset, bool flipX = false, bool flipY = false);

areatemplate_id get_area_template_by_name(const char* name);
AreaTemplate& get_area_template(areatemplate_id id);

// For use with data loading ONLY:
void clear_area_templates();
void add_area_template(AreaTemplate* area_template);

#endif /* AREATEMPLATE_H_ */
