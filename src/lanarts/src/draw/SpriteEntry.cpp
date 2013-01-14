#include "SpriteEntry.h"

sprite_id get_sprite_by_name(const char* name);

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

	sprite_id spriteid(const char* name) {
		return get_sprite_by_name(name);
	}

	sprite_id spriteid(const std::string& name) {
		return get_sprite_by_name(name.c_str());
	}
}

