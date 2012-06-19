/*
 * tunnelgen.h
 * Tunnel generation algorithms
 */

#ifndef TUNNELGEN_H_
#define TUNNELGEN_H_

const int MAX_TUNNEL_WIDTH = 4;

#include "../util/mtwist.h"
#include "GeneratedLevel.h"

struct TunnelGenSettings {
	int padding;
	Range size, num_tunnels;
};
void generate_entrance(const Region& r, MTwist& mt, int len, Pos& p, bool& axis,
		bool& more);
void generate_tunnels(const TunnelGenSettings& tgs, MTwist& mt, GeneratedLevel& level);

#endif /* TUNNELGEN_H_ */
