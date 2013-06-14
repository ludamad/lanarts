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

	struct TunnelSelector {
		Selector is_valid_fill, is_valid_perimeter;
		Selector is_finished_fill, is_finished_perimeter;

		TunnelSelector(Selector is_valid_fill, Selector is_valid_perimeter,
				Selector is_finished_fill, Selector is_finished_perimeter) :
						is_valid_fill(is_valid_fill),
						is_valid_perimeter(is_valid_perimeter),
						is_finished_fill(is_finished_fill),
						is_finished_perimeter(is_finished_perimeter) {
		}
	};

	struct TunnelGenOperator: public AreaOperatorBase {
		MTwist randomizer;
		TunnelSelector selector;
		ConditionalOperator fill_oper, perimeter_oper;
		int padding;
		Range size, num_tunnels;

		TunnelGenOperator(MTwist& randomizer, const TunnelSelector& selector,
				ConditionalOperator fill_oper,
				ConditionalOperator perimeter_oper, int padding, Range size,
				Range num_tunnels) :
						randomizer(randomizer),
						selector(selector),
						fill_oper(fill_oper),
						perimeter_oper(perimeter_oper),
						padding(padding),
						size(size),
						num_tunnels(num_tunnels) {
		}

		virtual bool apply(MapPtr map, group_t parent_group_id, const BBox& rect);
	};
}

#endif /* TUNNELGEN_H_ */
