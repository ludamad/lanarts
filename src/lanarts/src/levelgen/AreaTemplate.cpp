/*
 * AreaTemplate.cpp:
 *  Represents a partially defined area. Can be a standalone level template,
 *  or embedded as a small part of another level.
 *  They can be rotated randomly or not.
 */

#include <cstdio>

#include "AreaTemplate.h"
#include "GeneratedLevel.h"

AreaTemplate::AreaTemplate(const std::string& name, const char* data, int data_width, int width,
		int height, const std::vector<Glyph>& glyphs) :
		_name(name), _width(width), _height(height) {
	LANARTS_ASSERT(_width > 0);
	LANARTS_ASSERT(data_width >= _width);
	LANARTS_ASSERT(_height > 0);
	int len = _width * _height;
	_data.resize(len, '\0');
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int ind1 = y * data_width + x;
			char glyph = data[ind1];

			char result = EMPTY_GLYPH;
			if (glyph == 'x') {
				result = WALL_GLYPH;
			} else if (glyph == '-') {
				result = EMPTY_GLYPH;
			} else if (glyph == '>') {
				result = STAIRUP_GLYPH;
			} else if (glyph == '<') {
				result = STAIRDOWN_GLYPH;
			} else if (glyph == '+') {
				result = DOOR_GLYPH;
			}

			int ind2 = y * width + x;
			_data[ind2] = result;
		}
	}
}

AreaTemplate::~AreaTemplate() {
}

void generate_area(GeneratedLevel& level, AreaTemplate& area_template,
		const Pos& offset, bool flipX, bool flipY) {
	char* data = area_template.data();
	int w = area_template.width(), h = area_template.height();

	int roomid = level.rooms().size();
	Room room(Region(offset.x, offset.y, w, h), roomid);
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int ind = w * y + x;
			int sqrx = flipX ? w - x - 1 : x;
			int sqry = flipY ? h - y - 1 : y;
			Sqr& sqr = level.at(sqrx + offset.x, sqry + offset.y);
			sqr.groupID = roomid;
			sqr.roomID = roomid;
			if (data[ind] != AreaTemplate::WALL_GLYPH) {
				sqr.passable = true;
			} else {
				sqr.passable = false;
			}
			sqr.feature = NORMAL;
		}
	}

	level.rooms().push_back(room);
}

static std::vector<AreaTemplate*> area_template_data;

areatemplate_id get_area_template_by_name(const char* name) {
	for (int i = 0; i < area_template_data.size(); i++) {
		if (name == area_template_data.at(i)->name()) {
			return i;
		}
	}
	/*Error if resource not found*/
	fprintf(stderr, "Failed to load resource!\nname: %s, of type %s\n", name,
			"AreaTemplate");
	fflush(stderr);
	LANARTS_ASSERT(false /*resource not found*/);
	return -1;
}

AreaTemplate& get_area_template(areatemplate_id id) {
	return *area_template_data.at(id);
}

void clear_area_templates() {
	for (int i = 0; i < area_template_data.size(); i++) {
		delete area_template_data[i];
	}
	area_template_data.clear();
}

void add_area_template(AreaTemplate* area_template) {
	area_template_data.push_back(area_template);
}

