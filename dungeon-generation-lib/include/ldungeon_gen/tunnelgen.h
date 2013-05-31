/*
 * tunnelgen.h:
 *  Tunnel generation algorithms
 */

#ifndef TUNNELGEN_H_
#define TUNNELGEN_H_

const int MAX_TUNNEL_WIDTH = 4;

#include <lcommon/mtwist.h>
#include "Map.h"
#include "map_fill.h"
#include <string>

namespace ldungeon_gen {

	void generate_entrance(const BBox& bbox, MTwist& mt, int len, Pos& p,
			bool& axis, bool& more);

	struct TunnelGenOperator: public AreaOperatorBase {
		MTwist randomizer;
		Selector is_valid, is_finished;
		ConditionalOperator fill_oper, perimeter_oper;
		int padding;
		Range size, num_tunnels;

		TunnelGenOperator(MTwist& randomizer, Selector is_valid,
				Selector is_finished, ConditionalOperator fill_oper,
				ConditionalOperator perimeter_oper, int padding, Range size,
				Range num_tunnels) :
						randomizer(randomizer),
						is_valid(is_valid),
						is_finished(is_finished),
						fill_oper(fill_oper),
						perimeter_oper(perimeter_oper),
						padding(padding),
						size(size),
						num_tunnels(num_tunnels) {
		}

		virtual bool apply(Map& map, group_t parent_group_id, const BBox& rect);
	};
}

#endif /* TUNNELGEN_H_ */
