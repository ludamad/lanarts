/*
 * map_procedural.cpp:
 *  Seeded gradient noise, cellular automata and random walks over a map.
 */

#include <algorithm>
#include <cmath>

#include "map_procedural.h"

namespace ldungeon_gen {

	/*****************************************************************************
	 * Noise
	 *****************************************************************************/

	NoiseField::NoiseField(unsigned int seed, float scale, int octaves, float persistence,
			float lacunarity) :
			scale(std::max(scale, 0.001f)),
			persistence(persistence),
			lacunarity(lacunarity),
			octaves(std::max(octaves, 1)) {
		MTwist rng(seed);
		for (int i = 0; i < 256; i++) {
			perm[i] = i;
		}
		for (int i = 255; i > 0; i--) {
			std::swap(perm[i], perm[rng.rand(i + 1)]);
		}
		for (int i = 0; i < 256; i++) {
			perm[256 + i] = perm[i];
		}
	}

	static inline float fade(float t) {
		return t * t * t * (t * (t * 6 - 15) + 10);
	}

	static inline float lerp(float a, float b, float t) {
		return a + (b - a) * t;
	}

	// Dot product of the offset with one of 8 gradient directions picked by the hash
	static inline float grad(int hash, float x, float y) {
		switch (hash & 7) {
		case 0: return x + y;
		case 1: return x - y;
		case 2: return -x + y;
		case 3: return -x - y;
		case 4: return x;
		case 5: return -x;
		case 6: return y;
		default: return -y;
		}
	}

	float NoiseField::gradient_noise(float x, float y) const {
		int xi = int(std::floor(x)), yi = int(std::floor(y));
		float xf = x - xi, yf = y - yi;
		xi &= 255;
		yi &= 255;
		float u = fade(xf), v = fade(yf);
		int aa = perm[perm[xi] + yi], ab = perm[perm[xi] + yi + 1];
		int ba = perm[perm[xi + 1] + yi], bb = perm[perm[xi + 1] + yi + 1];
		float x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u);
		float x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);
		return lerp(x1, x2, v);
	}

	float NoiseField::at(float x, float y) const {
		float total = 0, amplitude = 1, frequency = 1.0f / scale, max_total = 0;
		for (int i = 0; i < octaves; i++) {
			total += gradient_noise(x * frequency, y * frequency) * amplitude;
			max_total += amplitude;
			amplitude *= persistence;
			frequency *= lacunarity;
		}
		// Gradient noise rarely leaves [-0.7, 0.7]; stretch to about [-1, 1]
		return std::max(-1.0f, std::min(1.0f, total / max_total * 1.4f));
	}

	float noise_falloff(float value, BBox area, float x, float y, float strength) {
		if (strength == 0) {
			return value;
		}
		float cx = (area.x1 + area.x2) / 2.0f, cy = (area.y1 + area.y2) / 2.0f;
		float nx = (x - cx) / std::max(1.0f, area.width() / 2.0f);
		float ny = (y - cy) / std::max(1.0f, area.height() / 2.0f);
		float d = std::min(1.0f, std::sqrt(nx * nx + ny * ny));
		return value - strength * 2 * d * d;
	}

	int noise_apply(Map& map, BBox area, const NoiseField& field, float falloff,
			const std::vector<NoiseBand>& bands) {
		int applied = 0;
		FOR_EACH_BBOX(area, x, y) {
			float value = noise_falloff(field.at(x, y), area, x, y, falloff);
			for (const NoiseBand& band : bands) {
				if (value <= band.max_value) {
					Square& sqr = map[Pos(x, y)];
					if (sqr.matches(band.oper.selector)) {
						sqr.apply(band.oper.oper);
						applied++;
					}
					break;
				}
			}
		}
		return applied;
	}

	/*****************************************************************************
	 * Cellular automata
	 *****************************************************************************/

	int automaton_step(Map& map, MTwist& rng, BBox area, int radius, bool outside_counts,
			const std::vector<AutomatonRule>& rules) {
		// Every square reads the map as it was before this step
		Map before = map;
		int changed = 0;
		FOR_EACH_BBOX(area, x, y) {
			const Square& old_sqr = before[Pos(x, y)];
			for (const AutomatonRule& rule : rules) {
				if (!old_sqr.matches(rule.selector)) {
					continue;
				}
				int count = 0;
				for (int dy = -radius; dy <= radius; dy++) {
					for (int dx = -radius; dx <= radius; dx++) {
						if (dx == 0 && dy == 0) {
							continue;
						}
						int nx = x + dx, ny = y + dy;
						if (nx < area.x1 || ny < area.y1 || nx >= area.x2 || ny >= area.y2) {
							count += outside_counts ? 1 : 0;
						} else if (before[Pos(nx, ny)].matches(rule.neighbour)) {
							count++;
						}
					}
				}
				if (count < rule.min_count || count > rule.max_count) {
					continue;
				}
				// The first rule that matches decides, whether or not its chance comes up
				if (rule.chance >= 1 || rng.rand(1000000) < int(rule.chance * 1000000)) {
					Square& sqr = map[Pos(x, y)];
					Square prior = sqr;
					sqr.apply(rule.oper);
					if (prior.flags != sqr.flags || prior.content != sqr.content || prior.group != sqr.group) {
						changed++;
					}
				}
				break;
			}
		}
		return changed;
	}

	/*****************************************************************************
	 * Random walks
	 *****************************************************************************/

	static void apply_brush(Map& map, BBox area, Pos center, int width, const ConditionalOperator& oper) {
		int lo = -(width - 1) / 2, hi = width / 2;
		for (int dy = lo; dy <= hi; dy++) {
			for (int dx = lo; dx <= hi; dx++) {
				Pos p(center.x + dx, center.y + dy);
				if (p.x >= area.x1 && p.y >= area.y1 && p.x < area.x2 && p.y < area.y2) {
					map[p].apply(oper);
				}
			}
		}
	}

	Pos random_walk_apply(Map& map, MTwist& rng, BBox area, const RandomWalk& walk) {
		static const Pos DIRECTIONS[4] = { Pos(1, 0), Pos(-1, 0), Pos(0, 1), Pos(0, -1) };
		Pos at = walk.from;
		apply_brush(map, area, at, walk.width, walk.oper);
		for (int i = 0; i < walk.steps; i++) {
			if (walk.has_target && at == walk.target) {
				break;
			}
			Pos step;
			if (walk.has_target && rng.rand(1000000) < int(walk.target_bias * 1000000)) {
				int dx = walk.target.x - at.x, dy = walk.target.y - at.y;
				// Step along the axis with the further distance to go, breaking ties at random
				bool along_x = std::abs(dx) > std::abs(dy) || (std::abs(dx) == std::abs(dy) && rng.rand(2));
				step = along_x ? Pos(dx > 0 ? 1 : -1, 0) : Pos(0, dy > 0 ? 1 : -1);
			} else {
				step = DIRECTIONS[rng.rand(4)];
			}
			Pos next(at.x + step.x, at.y + step.y);
			if (next.x < area.x1 || next.y < area.y1 || next.x >= area.x2 || next.y >= area.y2) {
				continue;
			}
			if (!map[next].matches(walk.passable)) {
				continue;
			}
			at = next;
			apply_brush(map, area, at, walk.width, walk.oper);
		}
		return at;
	}
}
