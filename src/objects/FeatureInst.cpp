/*
 * FeatureInst.cpp:
 *  Represents a feature on the floor
 */

#include <typeinfo>

#include <luawrap/luawrap.h>

#include <ldraw/DrawOptions.h>

#include "draw/colour_constants.h"
#include "draw/SpriteEntry.h"

#include "gamestate/GameState.h"
#include "gamestate/GameMapState.h"
#include "gamestate/GameTiles.h"

#include "stats/items/ItemEntry.h"

#include <lcommon/math_util.h>
#include "player/PlayerInst.h"
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
	    res::sprite(last_seen_spr).draw(
            ldraw::DrawOptions().origin(ldraw::CENTER).frame(sprite_frame).colour(
                    drawcolour), on_screen(gs, ipos()));
	    if (!used_yet) {
	        res::sprite("spr_effects.new_stair").draw(
	            ldraw::DrawOptions().origin(ldraw::CENTER), on_screen(gs, ipos()));
	    }
    }
}

void FeatureInst::step(GameState* gs) {
	if (gs->object_visible_test(this)) {
		sprite_id spr = spriteid;
		if (feature == DOOR_CLOSED) {
			spr = res::sprite_id("closed door");
		} else if (feature == DOOR_OPEN) {
			spr = res::sprite_id("open door");
		}
		last_seen_spr = spr;
	}

	Pos tile_xy(x / TILE_SIZE, y / TILE_SIZE);
	if (gs->object_radius_test(x, y, TILE_SIZE)) {
		if (feature == DOOR_CLOSED) {
			gs->tiles().set_solid(tile_xy, false);
			gs->tiles().set_seethrough(tile_xy, true);
			feature = DOOR_OPEN;
		}
	} else if (!gs->object_radius_test(x, y, TILE_SIZE)) {
		if (feature == DOOR_OPEN) {
			gs->tiles().set_solid(tile_xy, true);
			gs->tiles().set_seethrough(tile_xy, false);
			feature = DOOR_CLOSED;
		}
	}
}

void FeatureInst::init(GameState* gs) {
	GameInst::init(gs);
	if (feature == DOOR_CLOSED || solid) {
		Pos tile_xy(x / TILE_SIZE, y / TILE_SIZE);
		gs->tiles().set_solid(tile_xy, true);
		if (feature == DOOR_CLOSED) {
			gs->tiles().set_seethrough(tile_xy, false);
		}
	}
}

void FeatureInst::deinit(GameState *gs) {
	if (feature == DOOR_CLOSED || solid) {
		Pos tile_xy(x / TILE_SIZE, y / TILE_SIZE);
		gs->tiles().set_solid(tile_xy, false);
		if (feature == DOOR_CLOSED) {
			gs->tiles().set_seethrough(tile_xy, true);
		}
	}
}

void FeatureInst::player_interact(GameState* gs, GameInst* inst) {
	lua_State* L = gs->luastate();
	lua_lookup(L, "on_player_interact");
	used_yet = true;
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return;
	}
	luawrap::push(L, (GameInst*)this);
	luawrap::push(L, inst);
	lua_call(L, 2, 0);
	GameInst* post_inst = NULL;
	PlayerInst* p = gs->local_player();
	inst->get_map(gs)->object_radius_test(p, &post_inst, 1, feature_colfilter, p->x, p->y, p->radius);
	if (post_inst) {
	    dynamic_cast<FeatureInst*>(post_inst)->used_yet = true;
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

void FeatureInst::serialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::serialize(gs, serializer);
	serializer.write_int(feature);
	serializer.write_int(last_seen_spr);
	serializer.write_int(spriteid);
    serializer.write_int(sprite_frame);
    serializer.write_int(used_yet);
}

void FeatureInst::deserialize(GameState* gs, SerializeBuffer& serializer) {
	GameInst::deserialize(gs, serializer);
	serializer.read_int(feature);
	serializer.read_int(last_seen_spr);
	serializer.read_int(spriteid);
    serializer.read_int(sprite_frame);
    serializer.read_int(used_yet);
}
