/*
 * featuregen.cpp:
 *  Defines parameters for feature generation (eg tile generation, staircase placement),
 *  as well as the generate_features function
 */

#include "../gamestate/GameState.h"
#include "../gamestate/GameTiles.h"
#include "../gamestate/GameLevelState.h"

#include "../objects/FeatureInst.h"

#include "../display/tile_data.h"
#include "../gamestate/tileset_data.h"

#include "../util/math_util.h"

#include "featuregen.h"

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

void generate_features(const FeatureGenSettings& fs, MTwist& mt,
		GeneratedLevel& level, GameState* gs) {
	GameTiles& tiles = gs->tile_grid();
	TilesetEntry& tileset = game_tileset_data[randtileset(mt, fs.tilesets)];
	tiles.clear();

	int tw = tiles.tile_width(), th = tiles.tile_height();
	int lw = level.width(), lh = level.height();

	int start_x = (tw - lw) / 2;
	int start_y = (th - lh) / 2;

	int end_x = start_x + lw;
	int end_y = start_y + lh;

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			Sqr& s = level.at(x - start_x, y - start_y);
			tiles.set_solid(x, y, !s.passable);
			if (s.passable) {
				tiles.get(x, y) = rltile(mt, tileset.floor);
				if (s.roomID) {
//					if (s.marking)
// 					tiles[ind] = TILE_MESH_0+s.marking;
				} else if (s.feature == SMALL_CORRIDOR) {
					tiles.get(x, y) = rltile(mt, tileset.corridor);
				}
			} else {
				tiles.get(x, y) = rltile(mt, tileset.wall);
				if (s.feature == SMALL_CORRIDOR) {
					if (mt.rand(4) == 0) {
						tiles.get(x, y) = rltile(mt, tileset.altwall);
					}
				}
			}
		}
	}

	for (int i = 0; i < level.rooms().size(); i++) {
		if (gs->rng().rand(3) != 0)
			continue;
		Region r = level.rooms()[i].room_region;
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
					tiles.get(x + start_x, y + start_y) = rltile(mt,
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

		tiles.get(p.x, p.y) = rltile(mt, get_tile_by_name("stairs_down"));

		gs->get_level()->entrances.push_back(GameLevelPortal(p, Pos(0, 0)));
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

		tiles.get(p.x, p.y) = rltile(mt, get_tile_by_name("stairs_up"));
		gs->get_level()->exits.push_back(GameLevelPortal(p, Pos(0, 0)));
	}

	for (int i = 0; i < 4; i++) {
		for (int attempts = 0; attempts < 200; attempts++) {
			Pos fpos = generate_location(mt, level);
			if (level.at(fpos).feature != SMALL_CORRIDOR)
				continue;
			fpos.x += start_x;
			fpos.y += start_y;
			fpos = centered_multiple(fpos, TILE_SIZE);
			gs->add_instance(
					new FeatureInst(fpos.x, fpos.y, FeatureInst::DOOR_CLOSED));
			break;
		}
	}
}
