/*
 * tunnelgen.h:
 *  Tunnel generation algorithms
 */

#ifndef TUNNELGEN_H_
#define TUNNELGEN_H_

const int MAX_TUNNEL_WIDTH = 4;

#include <lcommon/mtwist.h>

#include "GeneratedRoom.h"

#include "dungeon_data.h"

void generate_entrance(const Region& r, MTwist& mt, int len, Pos& p, bool& axis,
		bool& more);
void generate_tunnels(const TunnelGenSettings& tgs, MTwist& mt, GeneratedRoom& level);

#endif /* TUNNELGEN_H_ */
