/*
 * FeatureInst.cpp:
 *  Represents a feature on the floor
 */

#include <typeinfo>

#include "collision_filters.h"
#include "../util/math_util.h"

#include "../stats/item_data.h"
#include "../display/colour_constants.h"
#include "../display/sprite_data.h"

#include "../gamestate/GameTiles.h"
#include "../gamestate/GameState.h"

#include "FeatureInst.h"

FeatureInst::FeatureInst(const Pos& pos, feature_t feature, bool solid,
		sprite_id spriteid, int sprite_frame) :
		GameInst(pos.x, pos.y, RADIUS, solid, DEPTH), feature(feature), last_seen_spr(
				-1), spriteid(spriteid), sprite_frame(sprite_frame) {

}

FeatureInst::~FeatureInst() {
}

void FeatureInst::draw(GameState* gs) {
	Colour drawcolour = COL_WHITE;
	if (last_seen_spr > -1) {
		SpriteEntry& spr_entry = game_sprite_data.at(spriteid);
		GLimage& img = spr_entry.img(sprite_frame);
		gl_draw_image(gs->view(), img, x - TILE_SIZE / 2, y - TILE_SIZE / 2,
				drawcolour);
	}
}

void FeatureInst::step(GameState* gs) {

	if (gs->object_visible_test(this)) {
		sprite_id spr = spriteid;
		if (feature == DOOR_CLOSED) {
			spr = get_sprite_by_name("closed door");
		} else if (feature == DOOR_OPEN) {
			spr = get_sprite_by_name("open door");
		}
		last_seen_spr = spr;
	}
	if (gs->object_radius_test(x, y, TILE_SIZE, player_colfilter)) {
		if (feature == DOOR_CLOSED) {
			gs->tiles().set_solid(x / TILE_SIZE, y / TILE_SIZE, false);
			feature = DOOR_OPEN;
		}
	} else if (!gs->object_radius_test(x, y, TILE_SIZE)) {
		if (feature == DOOR_OPEN) {
			gs->tiles().set_solid(x / TILE_SIZE, y / TILE_SIZE, true);
			feature = DOOR_CLOSED;
		}
	}
}

void FeatureInst::init(GameState* gs) {
	if (feature == DOOR_CLOSED || solid) {
		gs->tiles().set_solid(x / TILE_SIZE, y / TILE_SIZE, true);
	}
}

void FeatureInst::deinit(GameState *gs) {
	if (feature == DOOR_CLOSED || solid) {
		gs->tiles().set_solid(x / TILE_SIZE, y / TILE_SIZE, false);
	}
}

void FeatureInst::player_interact(GameState* gs) {
	if (feature == DOOR_CLOSED) {
		gs->tiles().set_solid(x / TILE_SIZE, y / TILE_SIZE, false);
		feature = DOOR_OPEN;
	}
}

void FeatureInst::copy_to(GameInst* inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(FeatureInst*)inst = *this;
}

FeatureInst* FeatureInst::clone() const {
	return new FeatureInst(*this);
}

bool feature_exists_near(GameState* gs, const Pos& p) {
	Pos midx = round_to_multiple(p, TILE_SIZE, true);
	return gs->object_radius_test(midx.x, midx.y, 4);
}

void FeatureInst::serialize(GameState *gs, SerializeBuffer & serializer) {
}

void FeatureInst::deserialize(GameState *gs, SerializeBuffer & serializer) {
}

