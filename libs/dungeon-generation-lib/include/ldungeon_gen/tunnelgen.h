/*
 * tunnelgen.h:
 *  Tunnel generation algorithms
 */

#ifndef LDUNGEON_TUNNELGEN_H_
#define LDUNGEON_TUNNELGEN_H_

#include <lcommon/mtwist.h>
#include "Map.h"
#include "map_fill.h"
#include <string>

namespace ldungeon_gen {

	void generate_entrance(const BBox& bbox, MTwist& mt, int len, Pos& p,
			bool& axis, bool& more);

	struct TunnelCheckSettings {
		Selector is_valid_fill, is_valid_perimeter;
		Selector is_finished_fill, is_finished_perimeter;

		Selector avoid_selector;
		Selector end_selector;

		TunnelCheckSettings(Selector is_valid_fill, Selector is_valid_perimeter,
				Selector is_finished_fill, Selector is_finished_perimeter,
				Selector avoid_selector = Selector(), Selector end_selector = Selector()) :
				is_valid_fill(is_valid_fill), is_valid_perimeter(
						is_valid_perimeter), is_finished_fill(is_finished_fill), is_finished_perimeter(
						is_finished_perimeter), avoid_selector(avoid_selector), end_selector(end_selector) {
		}
	};

	struct TunnelFillSettings {
		ConditionalOperator fill_oper;
		int padding;
		ConditionalOperator perimeter_oper;
		int width, max_length;
		float turn_chance;
		TunnelFillSettings(ConditionalOperator fill_oper, int padding,
				ConditionalOperator perimeter_oper, int width, int max_length,
				float turn_chance) :
				fill_oper(fill_oper), padding(padding), perimeter_oper(
						perimeter_oper), width(width), max_length(max_length), turn_chance(
						turn_chance) {
		}
	};

	struct TunnelGenOperator: public AreaOperatorBase {
		MTwist randomizer;
		TunnelCheckSettings checker;
		ConditionalOperator fill_oper, perimeter_oper;
		int padding;
		Range size, num_tunnels;

		TunnelGenOperator(MTwist& randomizer,
				const TunnelCheckSettings& checker,
				ConditionalOperator fill_oper,
				ConditionalOperator perimeter_oper, int padding, Range size,
				Range num_tunnels) :
				randomizer(randomizer), checker(checker), fill_oper(
						fill_oper), perimeter_oper(perimeter_oper), padding(
						padding), size(size), num_tunnels(num_tunnels) {
		}

		virtual bool apply(MapPtr map, group_t parent_group_id,
				const BBox& rect);

		void try_tunnel(MapPtr map, const BBox& root_rect, Selector root_selector, Selector destination_selector);
	};

	/* 'xy' determines where to start tunneling from.
	 * 'direction' determines what direction to tunnel from there */
	bool tunnel_generate(MapPtr map, MTwist& randomizer, const Pos& xy,
			const Pos& direction, const TunnelCheckSettings& checker,
			const TunnelFillSettings& filler);
}
#endif /* LDUNGEON_TUNNELGEN_H_ */
