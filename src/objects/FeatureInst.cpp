/*
 * FeatureInst.cpp:
 *  Represents a feature on the floor
 */

#include <typeinfo>

#include "../display/colour_constants.h"
#include "../display/sprite_data.h"

#include "../gamestate/GameState.h"
#include "../gamestate/GameTiles.h"
#include "../stats/item_data.h"

#include "../util/math_util.h"
#include "FeatureInst.h"

#include "collision_filters.h"

FeatureInst::FeatureInst(const Pos& pos, feature_t feature, bool solid,
		sprite_id spriteid, int depth, int sprite_frame) :
		GameInst(pos.x, pos.y, RADIUS, solid, depth), feature(feature), last_seen_spr(
				-1), spriteid(spriteid), sprite_frame(sprite_frame) {

}

FeatureInst::~FeatureInst() {
}

void FeatureInst::draw(GameState* gs) {
	Colour drawcolour = COL_WHITE;
	if (last_seen_spr > -1) {
		SpriteEntry& spr_entry = game_sprite_data.at(last_seen_spr);
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
	if (gs->object_radius_test(x, y, TILE_SIZE)) {
		if (feature == DOOR_CLOSED) {
			gs->tiles().set_solid(x / TILE_SIZE, y / TILE_SIZE, false);
			gs->tiles().set_seethrough(x / TILE_SIZE, y / TILE_SIZE, true);
			feature = DOOR_OPEN;
		}
	} else if (!gs->object_radius_test(x, y, TILE_SIZE)) {
		if (feature == DOOR_OPEN) {
			gs->tiles().set_solid(x / TILE_SIZE, y / TILE_SIZE, true);
			gs->tiles().set_seethrough(x / TILE_SIZE, y / TILE_SIZE, false);
			feature = DOOR_CLOSED;
		}
	}
}

void FeatureInst::init(GameState* gs) {
	if (feature == DOOR_CLOSED || solid) {
		gs->tiles().set_solid(x / TILE_SIZE, y / TILE_SIZE, true);
		if (feature == DOOR_CLOSED) {
			gs->tiles().set_seethrough(x / TILE_SIZE, y / TILE_SIZE, false);
		}
	}
}

void FeatureInst::deinit(GameState *gs) {
	if (feature == DOOR_CLOSED || solid) {
		gs->tiles().set_solid(x / TILE_SIZE, y / TILE_SIZE, false);
		if (feature == DOOR_CLOSED) {
			gs->tiles().set_seethrough(x / TILE_SIZE, y / TILE_SIZE, true);
		}
	}
}

void FeatureInst::player_interact(GameState* gs) {
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

void FeatureInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::serialize(gs, serializer);
	serializer.write_int(feature);
	serializer.write_int(last_seen_spr);
	serializer.write_int(spriteid);
	serializer.write_int(sprite_frame);
}

void FeatureInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::deserialize(gs, serializer);
	serializer.read_int(feature);
	serializer.read_int(last_seen_spr);
	serializer.read_int(spriteid);
	serializer.read_int(sprite_frame);
}

