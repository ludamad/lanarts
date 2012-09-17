/*
 * AreaTemplate.h:
 *  Represents a partially defined area. Can be a standalone level template,
 *  or embedded as a small part of another level.
 *  They can be rotated randomly or not.
 */

#ifndef AREATEMPLATE_H_
#define AREATEMPLATE_H_

#include <cstring>
#include <vector>

#include "../util/mtwist.h"
#include "../lanarts_defines.h"

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
	Glyph(char glyph, enemy_id enemy, const Item& item) :
			glyph(glyph), enemy(enemy), item(item) {
	}
};

class AreaTemplate {
public:
	enum {
		EMPTY_GLYPH, WALL_GLYPH, STAIRDOWN_GLYPH, STAIRUP_GLYPH, DOOR_GLYPH
	};
	AreaTemplate();
	void initialize(const char* data, int width, int height,
			const std::vector<Glyph>& glyphs);
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

private:
	std::vector<char> _data;
	int _width, _height;
	std::vector<MonsterGenCoord> _monsters;
};

void generate_area(GeneratedLevel& level, AreaTemplate& area_template,
		const Pos& offset);

#endif /* AREATEMPLATE_H_ */
