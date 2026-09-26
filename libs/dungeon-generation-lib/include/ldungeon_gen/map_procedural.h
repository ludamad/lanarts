/*
 * map_procedural.h:
 *  Procedural fields and processes over a map: seeded gradient noise, cellular automata and
 *  random walks. Each works on square flags/content through selectors and operators, so the same
 *  primitives serve terrain, forests, caves, rivers, trails and so on.
 */

#ifndef LDUNGEON_MAP_PROCEDURAL_H_
#define LDUNGEON_MAP_PROCEDURAL_H_

#include <vector>

#include <lcommon/mtwist.h>

#include "Map.h"

namespace ldungeon_gen {

	/* Fractal (fBm) 2D gradient noise, deterministic for a given seed. Values are roughly in [-1, 1]. */
	class NoiseField {
	public:
		NoiseField(unsigned int seed, float scale = 16, int octaves = 4, float persistence = 0.5f,
				float lacunarity = 2.0f);
		float at(float x, float y) const;
	private:
		float gradient_noise(float x, float y) const;
		int perm[512];
		float scale, persistence, lacunarity;
		int octaves;
	};

	/* Lowers noise values towards the edges of an area: 0 at the centre, 'strength' * 2 at the rim.
	 * Returns the adjusted value. */
	float noise_falloff(float value, BBox area, float x, float y, float strength);

	struct NoiseBand {
		float max_value; // applies to squares whose value is <= max_value (first matching band wins)
		ConditionalOperator oper;
	};

	/* For every square in area, samples the field and applies the first band whose max_value is at
	 * least the sample. Returns the number of squares a band applied to. */
	int noise_apply(Map& map, BBox area, const NoiseField& field, float falloff,
			const std::vector<NoiseBand>& bands);

	struct AutomatonRule {
		Selector selector;     // squares this rule can change
		Selector neighbour;    // which neighbours are counted
		int min_count, max_count; // applies when the neighbour count is within [min, max]
		float chance;          // probability of applying when it matches
		Operator oper;
	};

	/* One synchronous step of a cellular automaton: every square reads its neighbours (within
	 * 'radius', Moore neighbourhood) as they were before the step, and the first matching rule is
	 * applied. Squares outside the area count as neighbours when outside_counts is set.
	 * Returns the number of squares changed. */
	int automaton_step(Map& map, MTwist& rng, BBox area, int radius, bool outside_counts,
			const std::vector<AutomatonRule>& rules);

	struct RandomWalk {
		Pos from;
		int steps;
		Selector passable;     // squares the walker may enter
		ConditionalOperator oper; // applied to matching squares in a width x width brush at each position
		int width;
		bool has_target;
		Pos target;
		float target_bias;     // chance per step of moving towards the target instead of randomly
	};

	/* Walks from 'from' within area, applying the operator as it goes. Stops early on reaching the
	 * target. Returns where it ended. */
	Pos random_walk_apply(Map& map, MTwist& rng, BBox area, const RandomWalk& walk);
}

#endif /* LDUNGEON_MAP_PROCEDURAL_H_ */
