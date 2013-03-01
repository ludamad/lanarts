/*
 * featuregen.cpp:
 *  Defines parameters for feature generation (eg tile generation, staircase placement),
 *  as well as the generate_features function
 */

#include "data/game_data.h"

#include "draw/SpriteEntry.h"
#include "draw/TileEntry.h"
#include "gamestate/GameRoomState.h"

#include "gamestate/GameState.h"
#include "gamestate/GameTiles.h"

#include "gamestate/tileset_data.h"
#include "objects/store/StoreInst.h"
#include "objects/FeatureInst.h"

#include <lcommon/math_util.h>

#include "featuregen.h"
#include "itemgen.h"

/* Generate a random subtile for a tile */
static Tile rltile(MTwist& mt, int tile) {
	int subtiles = game_tile_data[tile].images.size();
	return Tile(tile, mt.rand(subtiles));
}

/* Generate a random tile from a tile range */
static Tile rltile(MTwist& mt, const Range& r) {
	return rltile(mt, mt.rand(r));
}

static tileset_id randtileset(MTwist& mt,
		const std::vector<tileset_id>& tilesets) {
	return tilesets[mt.rand(tilesets.size())];
}

static void create_door(GameState* gs, GeneratedRoom& l, int x, int y) {
	GameTiles& tiles = gs->tiles();
	int tw = tiles.tile_width(), th = tiles.tile_height();
	int lw = l.width(), lh = l.height();

	int start_x = (tw - lw) / 2;
	int start_y = (th - lh) / 2;

	Sqr& s = l.at(x, y);
	if (!s.has_instance && s.passable) {
		s.has_instance = true;
		Pos fpos(x + start_x, y + start_y);
		fpos = centered_multiple(fpos, TILE_SIZE);
		gs->add_instance(new FeatureInst(fpos, FeatureInst::DOOR_CLOSED));
	}
}
static void remove_wall(GameState* gs, GeneratedRoom& l, int x, int y) {
	GameTiles& tiles = gs->tiles();
	int tw = tiles.tile_width(), th = tiles.tile_height();
	int lw = l.width(), lh = l.height();

	int start_x = (tw - lw) / 2;
	int start_y = (th - lh) / 2;

	Sqr& s = l.at(x, y);
	if (!s.passable) {
		s.passable = true;
	}
}
static void create_doors_all_around(GameState* gs, GeneratedRoom& l,
		const BBox& region) {
	for (int x = region.x1; x < region.x2; x++) {
		create_door(gs, l, x, region.y1);
		create_door(gs, l, x, region.y2 - 1);
	}
	for (int y = region.y1; y < region.y2; y++) {
		create_door(gs, l, region.x1, y);
		create_door(gs, l, region.x2 - 1, y);
	}
}
static void remove_all_around(GameState* gs, GeneratedRoom& l,
		const BBox& region) {
	for (int x = region.x1 + 1; x < region.x2 - 1; x++) {
		remove_wall(gs, l, x, region.y1);
		remove_wall(gs, l, x, region.y2 - 1);
	}
	for (int y = region.y1 + 1; y < region.y2 - 1; y++) {
		remove_wall(gs, l, region.x1, y);
		remove_wall(gs, l, region.x2 - 1, y);
	}
}
static StoreInventory generate_shop_inventory(MTwist& mt, int itemn) {
	StoreInventory inv;
	itemgenlist_id itemgenlist = get_itemgenlist_by_name("Store Items");
	for (int i = 0; i < itemn; /*below*/) {
		const ItemGenChance& igc = generate_item_choice(mt, itemgenlist);
		int quantity = mt.rand(igc.quantity);
		Item item = Item(igc.itemtype, quantity);
		int cost = mt.rand(item.item_entry().shop_cost.multiply(quantity));
		if (cost > 0) {
			inv.add(item, cost);
			i++;
		}
	}
	return inv;
}
static void generate_shop(GameState* gs, GeneratedRoom& level, MTwist& mt,
		const Pos& p) {
	Pos worldpos = level.get_world_coordinate(p);
	level.at(p).has_instance = true;

	int itemn = mt.rand(Range(2, 14));
	gs->add_instance(
			new StoreInst(worldpos, false, res::spriteid("store"),
					generate_shop_inventory(mt, itemn)));

}
static void generate_statue(GameState* gs, GeneratedRoom& level, MTwist& mt,
		const Pos& p) {
	Pos worldpos = level.get_world_coordinate(p);
	level.at(p).has_instance = true;

	int itemn = mt.rand(Range(2, 10));
	sprite_id spriteid = res::spriteid("statue");
	SpriteEntry& statue_sprite = game_sprite_data.at(spriteid);
	ldraw::Drawable& sprite = res::sprite(spriteid);
	int nimages = (int)sprite.animation_duration();
	if (nimages <= 0) nimages = 1;
	int imgid = mt.rand(nimages);
	gs->add_instance(
			new FeatureInst(worldpos, FeatureInst::DECORATION, true, spriteid,
					FeatureInst::DEPTH, imgid));
}

void generate_features(const FeatureGenSettings& fs, MTwist& mt,
		GeneratedRoom& level, GameState* gs) {

	std::vector<RoomRegion>& rooms = level.rooms();
	const int nrooms = rooms.size();

	for (int i = 0; i < nrooms; i++) {
		Region& r = rooms[i].region;
		if (gs->rng().rand(100) == 0) {
			remove_all_around(gs, level,
					BBox(r.x - 1, r.y - 1, r.x + r.w + 1, r.y + r.h + 1));
		}
	}

	GameTiles& tiles = gs->tiles();
	TilesetEntry& tileset = game_tileset_data[randtileset(mt, fs.tilesets)];
	tiles.clear();

	int tw = tiles.tile_width(), th = tiles.tile_height();
	int lw = level.width(), lh = level.height();

	int start_x = (tw - lw) / 2;
	int start_y = (th - lh) / 2;

	int end_x = start_x + lw;
	int end_y = start_y + lh;

	for (int y = 0; y < th; y++) {
		for (int x = 0; x < tw; x++) {
			Pos xy(x, y);

			if (y < start_y || y >= end_y || x < start_x || x >= end_x) {
				tiles.set_solid(xy, true);
				tiles.set_seethrough(xy, false);
				tiles.get(xy) = rltile(mt, tileset.wall);
				continue;
			}

			Sqr& s = level.at(x - start_x, y - start_y);
			tiles.set_solid(xy, !s.passable);
			tiles.set_seethrough(xy, s.passable);
			if (s.passable) {
				tiles.get(xy) = rltile(mt, tileset.floor);
				if (s.roomID) {
//					if (s.marking)
// 					tiles[ind] = TILE_MESH_0+s.marking;
				} else if (s.feature == SMALL_CORRIDOR) {
					tiles.get(xy) = rltile(mt, tileset.corridor);
				}
			} else {
				tiles.get(xy) = rltile(mt, tileset.wall);
				if (s.feature == SMALL_CORRIDOR) {
					if (mt.rand(4) == 0) {
						tiles.get(xy) = rltile(mt, tileset.altwall);
					}
				}
			}
		}
	}

	for (int i = 0; i < level.rooms().size(); i++) {
		if (gs->rng().rand(3) != 0)
			continue;
		Region r = level.rooms()[i].region;
		/*

		 int rx = r.x + gs->rng().rand(1, r.w-1);
		 int ry = r.y + gs->rng().rand(1, r.h-1);
		 int rw = gs->rng().rand(r.w);
		 int rh = gs->rng().rand(r.h);

		 if (rx + rw >= r.x+r.w) rw = r.x+r.w - 1 - rx;
		 if (ry + rh >= r.y+r.h) rh = r.y+r.h - 1 - ry;*/
		int rx = r.x, rw = r.w;
		int ry = r.y, rh = r.h;

		for (int y = ry; y < ry + rh; y++) {
			for (int x = rx; x < rx + rw; x++) {
				Sqr& s = level.at(x, y);
				if (s.passable && s.roomID && s.feature != SMALL_CORRIDOR)
					tiles.get(Pos(x + start_x, y + start_y)) = rltile(mt,
							tileset.altfloor);
			}
		}
	}
	gs->get_level()->entrances.clear();
	for (int n = 0; n < fs.nstairs_down; n++) {
		Pos p = generate_location(mt, level);
		level.at(p).has_instance = true;

		p.x += start_x;
		p.y += start_y;

		tiles.get(p) = rltile(mt, res::tileid("stairs_down"));

		gs->get_level()->entrances.push_back(GameRoomPortal(p, Pos(0, 0)));
	}

	gs->get_level()->exits.clear();
	for (int n = 0; n < fs.nstairs_up; n++) {
		Pos p = generate_location(mt, level);
		level.at(p).has_instance = true;

		int x = p.x - 4, y = p.y - 4;
		int ex = p.x + 5, ey = p.y + 5;
		x = std::max(0, x), y = std::max(0, y);
		ex = std::min(ex, level.width()), ey = std::min(ey, level.height());
		for (int yy = y; yy < ey; yy++) {
			for (int xx = x; xx < ex; xx++) {
				level.at(xx, yy).near_entrance = true;
			}
		}

		p.x += start_x;
		p.y += start_y;

		tiles.get(p) = rltile(mt, res::tileid("stairs_up"));
		gs->get_level()->exits.push_back(GameRoomPortal(p, Pos(0, 0)));
	}

	for (int i = 0; i < nrooms; i++) {
		Region& r = rooms[i].region;
		if (mt.rand(100) == 0) {
			create_doors_all_around(gs, level,
					BBox(r.x - 1, r.y - 1, r.x + r.w + 1, r.y + r.h + 1));
		}
	}

	int amount_statues = mt.rand(10);
	for (int attempts = 0; attempts < amount_statues; attempts++) {
		int ind = mt.rand(rooms.size());
		RoomRegion& r1 = rooms[ind];
		Region inner = r1.region.remove_perimeter();
		if (inner.w < 2 || inner.h < 2) {
			continue;
		}
		Pos pos = generate_location_in_region(mt, level, inner);
		Sqr& sqr = level.at(pos);
		if (!sqr.passable || sqr.has_instance) {
			continue;
		}
		generate_statue(gs, level, mt, pos);
	}

	if (mt.rand(4) == 0) {
		for (int attempts = 0; attempts < 200; attempts++) {
			Pos pos = generate_location(mt, level);
			Sqr& sqr = level.at(pos);
			if (!sqr.passable || sqr.has_instance) {
				continue;
			}
			generate_shop(gs, level, mt, pos);
			break;
		}
	}
}
