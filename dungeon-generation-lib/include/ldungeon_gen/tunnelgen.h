/*
 * tunnelgen.h:
 *  Tunnel generation algorithms
 */

#ifndef TUNNELGEN_H_
#define TUNNELGEN_H_

const int MAX_TUNNEL_WIDTH = 4;

#include <lcommon/mtwist.h>
#include "Map.h"
#include <string>

namespace ldungeon_gen {

	struct TunnelGenSettings {
		int padding;
		Range size, num_tunnels;
		TunnelGenSettings(int padding, Range size, Range num_tunnels) :
						padding(padding),
						size(size),
						num_tunnels(num_tunnels) {
		}
	};

	void generate_entrance(const BBox& bbox, MTwist& mt, int len, Pos& p,
			bool& axis, bool& more);
	void generate_tunnels(Map& map, const TunnelGenSettings& tgs, MTwist& mt,
			ConditionalOperator fill_oper, ConditionalOperator perimeter_oper);
}

#endif /* TUNNELGEN_H_ */
