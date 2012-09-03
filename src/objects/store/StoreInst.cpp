/*
 * StoreInst.cpp:
 *  Represents a store NPC or store tile
 */
#include <typeinfo>

#include "../../display/colour_constants.h"

#include "../../display/sprite_data.h"
#include "../../gamestate/GameState.h"

#include "../../serialize/SerializeBuffer.h"

#include "../../stats/item_data.h"
#include "../../util/math_util.h"
#include "../collision_filters.h"

#include "StoreInst.h"

void StoreInst::step(GameState* gs) {
	if (gs->object_visible_test(this)) {
		last_seen_spr = spriteid;
	}
	GameInst* player = NULL;
	gs->object_radius_test(this, &player, 1, player_colfilter, x, y, 24);
	if (player == (GameInst*)gs->local_player()) {
		gs->game_hud().override_sidebar_contents(&sidebar_display);
	}
}

void StoreInst::draw(GameState* gs) {
	Colour drawcolour;
	if (gs->object_radius_test(this, NULL, 0, player_colfilter, x, y, 24)) {
		drawcolour = Colour(255, 255, 100, 255);
	}
	if (last_seen_spr > -1) {
		gl_draw_sprite(gs->view(), last_seen_spr, x - TILE_SIZE / 2,
				y - TILE_SIZE / 2, drawcolour);
	}
}

void StoreInst::copy_to(GameInst* inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(StoreInst*)inst = *this;
}

void StoreInst::init(GameState* gs) {
	sidebar_display.init(this, gs->game_hud().sidebar_content_area());
}

StoreInst* StoreInst::clone() const {
	return new StoreInst(*this);
}

void StoreInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::serialize(gs, serializer);
	inv.serialize(serializer);
	serializer.write(last_seen_spr);
	serializer.write(spriteid);
}

void StoreInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::deserialize(gs, serializer);
	sidebar_display.init(this, gs->game_hud().sidebar_content_area());
	inv.deserialize(serializer);
	serializer.read(last_seen_spr);
	serializer.read(spriteid);
}

