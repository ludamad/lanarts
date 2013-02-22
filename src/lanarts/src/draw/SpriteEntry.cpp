#include "SpriteEntry.h"

sprite_id get_sprite_by_name(const char* name, bool error_if_not_found = false);

namespace res {
	const std::string& sprite_name(sprite_id idx) {
		return game_sprite_data.at(idx).name;
	}

	bool sprite_exists(const std::string& name) {
		return get_sprite_by_name(name.c_str()) != -1;
	}

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

