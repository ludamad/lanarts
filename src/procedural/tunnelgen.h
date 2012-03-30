/*
 * tunnelgen.h
 * Tunnel generation algorithms
 */

#ifndef TUNNELGEN_H_
#define TUNNELGEN_H_

const int MAX_TUNNEL_WIDTH = 4;

#include "mtwist.h"
#include "GeneratedLevel.h"

struct TunnelGenSettings {
	int padding;
	int minwidth, maxwidth;
	int min_tunnels, max_tunnels;
	TunnelGenSettings(int padding, int minw,int maxw, int minn, int maxn) :
			padding(padding), minwidth(minw), maxwidth(maxw),
			min_tunnels(minn), max_tunnels(maxn) {
	}
};
void generate_entrance(const Region& r, MTwist& mt, int len, Pos& p, bool& axis,
		bool& more);
void generate_tunnels(const TunnelGenSettings& tgs, MTwist& mt, GeneratedLevel& level);

#endif /* TUNNELGEN_H_ */
