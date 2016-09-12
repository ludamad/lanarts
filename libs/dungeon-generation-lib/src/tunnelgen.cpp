/*
 * tunnelgen.cpp:
 *  Tunnel generation algorithms
 */

#include <cassert>
#include <vector>
#include <cstring>

#include <lcommon/mtwist.h>
#include <lcommon/math_util.h>

#include "ldungeon_assert.h"
#include "tunnelgen.h"

namespace ldungeon_gen {

	struct TunnelSliceContext;

	/*Internal implementation class to connect a specified room to either another tunnel
	 * (accept_tunnel_entry = true) or to another room (always). */
	class TunnelGenImpl {
	public:
		enum {
			NO_TURN = 0, TURN_PERIMETER = 1, TURN_START = 2
		};
		TunnelGenImpl(Map& map, MTwist& mt, const TunnelCheckSettings& checker,
				const TunnelFillSettings& filler) :
						map(map),
						mt(mt),
						checker(checker),
						filler(filler) {
		}

	public:
		//Tunnel generation function, generates as described above
		bool generate(Pos p, int dx, int dy, std::vector<Square>& btbuff,
				std::vector<TunnelSliceContext>& tsbuff);

	private:
		Pos next_turn_position(TunnelSliceContext* cntxt, int& ndx, int& ndy);
		Pos next_tunnel_position(TunnelSliceContext* cntxt);

		void initialize_slice(TunnelSliceContext* cntxt, int turn_state, int dx,
				int dy, const Pos& pos);
		void backtrack_slice(Square* prev_content, TunnelSliceContext* cntxt,
				int dep);
		void tunnel_turning_slice(TunnelSliceContext* curr,
				TunnelSliceContext* next);
		void tunnel_straight_slice(TunnelSliceContext* cntxt,
				TunnelSliceContext* next);

		//all return true if valid
		bool validate_slice(Square* prev_content, TunnelSliceContext* cntxt,
				int dep);

	private:
		Map& map;
		MTwist& mt;
		TunnelCheckSettings checker;
		TunnelFillSettings filler;
	};

	static Square* backtrack_entry(Square* backtracking, int entry_size,
			int entryn) {
		return &backtracking[entry_size * entryn];
	}

	struct TunnelSliceContext {
		//provided
		Pos pos;
		int dx, dy;
		char turn_state;
		int attempt_number;

		//calculated
		bool tunneled;
		Pos perim_pos, newpos;
	};

	void TunnelGenImpl::initialize_slice(TunnelSliceContext* cntxt,
			int turn_state, int dx, int dy, const Pos& pos) {
		cntxt->turn_state = turn_state;
		cntxt->dx = dx, cntxt->dy = dy;
		cntxt->pos = pos;
		cntxt->perim_pos = Pos(pos.x - (dy != 0 ? filler.padding : 0),
				pos.y - (dx != 0 ? filler.padding : 0));
		cntxt->attempt_number = 0;
	}

	void TunnelGenImpl::backtrack_slice(Square* prev_content,
			TunnelSliceContext* cntxt, int dep) {
		int dx = cntxt->dx, dy = cntxt->dy;

		for (int i = 0; i < filler.width + filler.padding * 2; i++) {
			int xcomp = (dy == 0 ? 0 : i);
			int ycomp = (dx == 0 ? 0 : i);
			map[Pos(cntxt->perim_pos.x + xcomp, cntxt->perim_pos.y + ycomp)] =
					prev_content[i];
		}

	}
	bool TunnelGenImpl::validate_slice(Square* prev_content,
			TunnelSliceContext* cntxt, int dep) {
		int dx = cntxt->dx, dy = cntxt->dy;
		if (cntxt->perim_pos.x < 0
				|| cntxt->pos.x
						>= map.width() - (dy == 0 ? 0 : filler.width + filler.padding * 2))
			return false;
		if (cntxt->perim_pos.y < 0
				|| cntxt->pos.y
						>= map.height() -(dx == 0 ? 0 :  filler.width + filler.padding * 2))
			return false;

		/* Start with the assumption that we're done */
		cntxt->tunneled = true;

		int last_square = filler.width + filler.padding * 2 - 1;
		for (int i = 0; i <= last_square; i++) {
			int xcomp = (dy == 0 ? 0 : i);
			int ycomp = (dx == 0 ? 0 : i);
			Square& sqr = map[Pos(cntxt->pos.x + xcomp, cntxt->pos.y + ycomp)];

			/* Look for reasons we would not be done */
			bool is_perimeter = (i == 0 || i == last_square);
			Selector is_finished = is_perimeter ? checker.is_finished_perimeter : checker.is_finished_fill;
			if (!sqr.matches(is_finished)) {
				cntxt->tunneled = false;
				break;
			}
			if (sqr.group == checker.avoid_group || (checker.end_group >= 0 && sqr.group != checker.end_group)) {
				cntxt->tunneled = false;
				break;
			}
		}

		if (cntxt->tunneled) {
			/* Do not finish tunnelling in the middle of a turn */
			if (cntxt->turn_state != NO_TURN)
				return false;
			printf("TUNNELED!\n");
			return true;
		}

		for (int i = 0; i <= last_square; i++) {
			int xcomp = (dy == 0 ? 0 : i);
			int ycomp = (dx == 0 ? 0 : i);
			Square& sqr = map[Pos(cntxt->perim_pos.x + xcomp, cntxt->perim_pos.y + ycomp)];
			bool is_perimeter = (i == 0 || i == last_square);
			Selector is_valid = is_perimeter ? checker.is_valid_perimeter : checker.is_valid_fill;
			if (cntxt->turn_state == NO_TURN && !sqr.matches(is_valid)) {
				return false;
			}
			memcpy(prev_content + i, &sqr, sizeof(Square));
		}
		return true;
	}

	Pos TunnelGenImpl::next_turn_position(TunnelSliceContext* cntxt, int& ndx,
			int& ndy) {
		Pos newpos;
		int dx = cntxt->dx, dy = cntxt->dy;
		bool positive = mt.rand(2);

		if (dx == 0) {
			if (positive)
				newpos.x = cntxt->pos.x + filler.width;
			else
				newpos.x = cntxt->pos.x - 1;
			ndx = positive ? +1 : -1;
		} else {
			ndx = 0;
			newpos.x = dx > 0 ? cntxt->pos.x - filler.width : cntxt->pos.x + 1;
		}

		if (dy == 0) {
			if (positive)
				newpos.y = cntxt->pos.y + filler.width;
			else
				newpos.y = cntxt->pos.y - 1;
			ndy = positive ? +1 : -1;
		} else {
			ndy = 0;
			newpos.y = dy > 0 ? cntxt->pos.y - filler.width : cntxt->pos.y + 1;
		}
		return newpos;
	}

	Pos TunnelGenImpl::next_tunnel_position(TunnelSliceContext* cntxt) {
		Pos newpos;
		int dx = cntxt->dx, dy = cntxt->dy;

		if (dy == 0)
			newpos.y = cntxt->pos.y;
		else if (dy > 0)
			newpos.y = cntxt->pos.y + 1;
		else
			newpos.y = cntxt->pos.y - 1;

		if (dx == 0)
			newpos.x = cntxt->pos.x;
		else if (dx > 0)
			newpos.x = cntxt->pos.x + 1;
		else
			newpos.x = cntxt->pos.x - 1;

		return newpos;
	}

	void TunnelGenImpl::tunnel_straight_slice(TunnelSliceContext* cntxt,
			TunnelSliceContext* next) {
		int dx = cntxt->dx, dy = cntxt->dy;

		/* Make the first piece of perimeter */
		for (int i = 0; i < filler.padding; i++) {
			int xcomp = (dy == 0 ? 0 : i);
			int ycomp = (dx == 0 ? 0 : i);
			map[Pos(cntxt->perim_pos.x+xcomp, cntxt->perim_pos.y+ycomp)].apply(filler.perimeter_oper);
		}
		/* Make the inner fill */
		for (int i = 0; i < filler.width; i++) {
			int xcomp = (dy == 0 ? 0 : i);
			int ycomp = (dx == 0 ? 0 : i);
			map[Pos(cntxt->pos.x + xcomp, cntxt->pos.y + ycomp)].apply(filler.fill_oper);
		}

		for (int i = 0; i < filler.padding; i++) {
			int x = cntxt->perim_pos.x + (dy == 0 ? 0 : filler.width + i+filler.padding);
			int y = cntxt->perim_pos.y + (dx == 0 ? 0 : filler.width + i+filler.padding);
			map[Pos(x,y)].apply(filler.perimeter_oper);
		}

		Pos newpos = next_tunnel_position(cntxt);

		bool start_turn = cntxt->turn_state == NO_TURN
				&& (mt.randf() < filler.turn_chance);
		initialize_slice(next, start_turn ? TURN_START : NO_TURN, dx, dy,
				newpos);
	}
	void TunnelGenImpl::tunnel_turning_slice(TunnelSliceContext* cntxt,
			TunnelSliceContext* next) {
		int dx = cntxt->dx, dy = cntxt->dy;
		int ndx, ndy;
		Pos newpos = next_turn_position(cntxt, ndx, ndy);

		if (filler.padding > 0) { // TODO: Handle padding > 1 properly
			for (int i = 0; i < filler.width + filler.padding * 2; i++) {
				int xcomp = (dy == 0 ? 0 : i);
				int ycomp = (dx == 0 ? 0 : i);
				Square& sqr = map[Pos(cntxt->perim_pos.x + xcomp, cntxt->perim_pos.y + ycomp)];
				sqr.apply(filler.perimeter_oper);
			}
		}
		initialize_slice(next, TURN_PERIMETER, ndx, ndy, newpos);
	}

	template<class T>
	void __resizebuff(T& t, size_t size) {
		if (t.size() <= size / 2)
			t.resize(size);
		else if (t.size() < size)
			t.resize(t.size() * 2);
	}

	bool TunnelGenOperator::apply(MapPtr map, group_t parent_group_id,
			const BBox& rect) {

		Pos p;
		bool axis, positive;

		std::vector<Square> btbuff;
		std::vector<TunnelSliceContext> tsbuff;

		std::vector<int> genpaths(map->groups.size(), 0);
		std::vector<int> totalpaths(map->groups.size(), 0);
		for (int i = 0; i < map->groups.size(); i++) {
			totalpaths[i] = randomizer.rand(num_tunnels);
		}
		int nogen_tries = 0;
		while (nogen_tries < 2000) {
			nogen_tries++;

			for (int i = 0; i < map->groups.size(); i++) {
			    Group& group = map->groups[i];
			    if (!group.child_group_ids.empty()) {
			        continue;
			    }
				if (genpaths[i] >= totalpaths[i])
					continue;
				bool generated = false;
				int genwidth = randomizer.rand(size);
				for (; genwidth >= 1 && !generated; genwidth--) {
					int path_len = 2;
					for (int attempts = 0; attempts < 16 && !generated;
							attempts++) {
						checker.avoid_group = i;
						TunnelFillSettings filler(fill_oper, padding, perimeter_oper, genwidth, path_len, 0.05);
						TunnelGenImpl tg(*map, randomizer, checker, filler);

//						FOR_EACH_BBOX(group.group_area, xx, yy) {
//						    (*map)[Pos(xx,yy)].flags |= FLAG_RESERVED1;
//						}
						generate_entrance(group.group_area, randomizer,
								std::min(genwidth, 2), p, axis, positive);
						if (!rect.contains(p)) {
						    goto label_give_up_on_group;
						}

						int val = positive ? +1 : -1;
						int dx = axis ? 0 : val, dy = axis ? val : 0;

						if (tg.generate(p, dx, dy, btbuff, tsbuff)) {
							genpaths[i]++;
							nogen_tries = 0;
							path_len = 2;
							generated = true;
						}
						if (attempts >= 4) {
							path_len += 5;
						}
					}
				}
				label_give_up_on_group:;
			}
		}

		return true;
	}
	bool TunnelGenImpl::generate(Pos p, int dx, int dy,
			std::vector<Square>& btbuff,
			std::vector<TunnelSliceContext>& tsbuff) {

		int entry_size = filler.width + filler.padding * 2;

		__resizebuff(btbuff, entry_size * filler.max_length);
		__resizebuff(tsbuff, filler.max_length);

		Square* backtracking = &btbuff[0];
		TunnelSliceContext* tsc = &tsbuff[0];

		Square* prev_content;
		TunnelSliceContext* cntxt;

		bool complete_tunnel = false;
		int tunnel_depth = 0;

		//By setting TURN_PERIMETER we avoid trying a turn on the first tunnel slice
		initialize_slice(&tsc[tunnel_depth], TURN_PERIMETER, dx, dy, p);
		while (true) {
			prev_content = backtrack_entry(backtracking, entry_size,
					tunnel_depth);
			cntxt = &tsc[tunnel_depth];

			//We must leave room to initialize the next tunnel depth
			bool max_length_hit = tunnel_depth >= filler.max_length - 1;
			bool valid = !max_length_hit
							&& (cntxt->attempt_number > 0
									|| validate_slice(prev_content, cntxt,
											tunnel_depth));
			if (valid && cntxt->attempt_number <= 0) {

				if (cntxt->tunneled) {
					complete_tunnel = true;
					break;
				}

				if (cntxt->turn_state == TURN_START) {
					this->tunnel_turning_slice(cntxt, &tsc[tunnel_depth + 1]);
				} else {
					this->tunnel_straight_slice(cntxt, &tsc[tunnel_depth + 1]);
				}
				cntxt->attempt_number++;
				tunnel_depth++;
			} else {
				tunnel_depth--;
				if (tunnel_depth < 0)
					break;

				//set values to those of previous depth
				prev_content = backtrack_entry(backtracking, entry_size,
						tunnel_depth);
				cntxt = &tsc[tunnel_depth];

				backtrack_slice(prev_content, cntxt, tunnel_depth);
			}
		}

		return complete_tunnel;
	}

	void generate_entrance(const BBox& bbox, MTwist& mt, int len, Pos& p,
			bool& axis, bool& positive) {
		int ind;
		axis = mt.rand(2), positive = mt.rand(2);
		if (axis) {
			int rmx = bbox.x2 - len;
			if (rmx == bbox.x1 + 1)
				ind = rmx;
			else
				ind = mt.rand(bbox.x1 + 1, rmx);
			p.y = positive ? bbox.y2 : bbox.y1 - 1;
			p.x = ind;
		} else {
			int rmy = bbox.y2 - len;
			if (rmy == bbox.y1 + 1)
				ind = rmy;
			else
				ind = mt.rand(bbox.y1 + 1, rmy);
			p.x = positive ? bbox.x2 : bbox.x1 - 1;
			p.y = ind;
		}
	}

	bool tunnel_generate(MapPtr map, MTwist& randomizer, const Pos& xy,
			const Pos& direction, const TunnelCheckSettings& checker,
			const TunnelFillSettings& filler) {
		/* Assert some (very) conservative range */
		LDUNGEON_ASSERT(filler.width > 0 && filler.width <= 255);
		LDUNGEON_ASSERT(direction != Pos());

		/* TODO: Cache this. */
		std::vector<Square> btbuff;
		std::vector<TunnelSliceContext> tsbuff;

		TunnelGenImpl tg(*map, randomizer, checker, filler);
		return tg.generate(xy, signum(direction.x), signum(direction.y), btbuff, tsbuff);
	}
}
