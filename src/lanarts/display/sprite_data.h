#ifndef SPRITE_DATA_H_
#define SPRITE_DATA_H_

#include <vector>
#include <string>

#include <common/geometry.h>

#include "../data/FilenameList.h"//For FilenameList

#include "../lanarts_defines.h"

#include "GLImage.h"

struct SpriteEntry {
	enum sprite_type {
		ANIMATED, DIRECTIONAL
	};
	sprite_type type;
	Colour drawcolour;
	std::string name;
	std::vector<GLimage> images;

	int width() {
		return img().width;
	}

	int height() {
		return img().height;
	}
	Dim size() {
		return Dim(width(), height());
	}

	SpriteEntry(const std::string& name, const FilenameList& filenames,
			sprite_type type = ANIMATED, Colour drawcolour = Colour()) :
			type(type), name(name), drawcolour(drawcolour) {
		for (int i = 0; i < filenames.size(); i++) {
			images.push_back(GLimage(filenames[i]));
		}
	}
	void init() {
		for (int i = 0; i < images.size(); i++) {
			images[i].init();
		}
	}
	GLimage& img(int ind = 0) {
		return images.at(ind);
	}
};

extern std::vector<SpriteEntry> game_sprite_data;

sprite_id get_sprite_by_name(const char* name);

#endif /* SPRITE_DATA_H_ */
