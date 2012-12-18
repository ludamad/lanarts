#include "SpriteEntry.h"

namespace res {
	ldraw::Drawable& sprite(sprite_id idx) {
		return game_sprite_data.at(idx).sprite;
	}

	ldraw::Drawable& sprite(const char* name) {
		return sprite(get_sprite_by_name(name));
	}
	ldraw::Drawable& sprite(const std::string& name) {
		return sprite(name.c_str());
	}
}
