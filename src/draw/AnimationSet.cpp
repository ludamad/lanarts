/*
 * AnimationSet.cpp:
 *  Per-state animation clips for a character sprite.
 */

#include <unordered_map>

#include <luawrap/luawrap.h>

#include "SpriteEntry.h"
#include "AnimationSet.h"

static std::unordered_map<sprite_id, AnimationSet> animation_sets;

int AnimationClip::total_steps() const {
	int total = 0;
	for (int steps : frame_steps) {
		total += steps;
	}
	return total;
}

int AnimationClip::frame_at(int elapsed) const {
	int total = total_steps();
	if (total <= 0) {
		return 0;
	}
	if (loop) {
		elapsed %= total;
	} else if (elapsed >= total) {
		return frame_steps.size() - 1;
	}
	for (int i = 0; i < frame_steps.size(); i++) {
		if (elapsed < frame_steps[i]) {
			return i;
		}
		elapsed -= frame_steps[i];
	}
	return frame_steps.size() - 1;
}

static AnimationClip parse_clip(const LuaField& field) {
	AnimationClip clip;
	if (field.isnil()) {
		return clip;
	}
	clip.sprite = res::sprite_id(field["sprite"].to_str());
	LuaField frames = field["frames"];
	for (int i = 1; i <= frames.objlen(); i++) {
		clip.frame_steps.push_back(frames[i].to_int());
	}
	clip.loop = luawrap::defaulted(field, "loop", false);
	return clip;
}

void lapi_data_create_animation_set(const LuaStackValue& table) {
	AnimationSet set;
	set.idle = parse_clip(table["idle"]);
	set.walk = parse_clip(table["walk"]);
	set.attack = parse_clip(table["attack"]);
	set.cast = parse_clip(table["cast"]);
	set.hurt = parse_clip(table["hurt"]);
	set.death = parse_clip(table["death"]);
	animation_sets[res::sprite_id(table["sprite"].to_str())] = set;
}

void clear_animation_sets() {
	animation_sets.clear();
}

namespace res {
	const AnimationSet* animation_set(::sprite_id sprite) {
		auto it = animation_sets.find(sprite);
		return it == animation_sets.end() ? nullptr : &it->second;
	}
}
