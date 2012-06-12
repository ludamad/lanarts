/*
 * tunnelgen.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#include "tunnelgen.h"
#include "../util/mtwist.h"
#include <cassert>
#include <vector>

static const int MAXPADDING = 3;
static const int MAXWIDTH = 10;
static const int MAXSIZE = MAXWIDTH + MAXPADDING * 2;

#define LANARTS_ASSERT(x) assert(x)

struct TunnelSliceContext;
//defined below
void generate_entrance(const Region& r, MTwist& mt, int len, Pos& p, bool& axis,
		bool& positive); //defined below

/*Internal implementation class to connect a specified room to either another tunnel
 * (accept_tunnel_entry = true) or to another room (always). */
class TunnelGenImpl {
public:
	enum {
		NO_TURN = 0, TURN_PERIMETER = 1, TURN_START = 2
	};
	TunnelGenImpl(GeneratedLevel& s, MTwist& mt, int start_room, int padding,
			int width, int depth, int change_odds, bool ate = false) :
			s(s), mt(mt), start_room(start_room), end_room(0), padding(padding), accept_tunnel_entry(
					ate), avoid_groupid(0), width(width), maxdepth(depth), change_odds(
					change_odds) {
		LANARTS_ASSERT(padding <= MAXPADDING);
		LANARTS_ASSERT(width <= MAXWIDTH);
	}

public:
	//Tunnel generation function, generates as described above
	bool generate(Pos p, int dx, int dy, std::vector<Sqr>& btbuff,
			std::vector<TunnelSliceContext>& tsbuff);

private:
	Pos next_turn_position(TunnelSliceContext* cntxt, int& ndx, int& ndy);
	Pos next_tunnel_position(TunnelSliceContext* cntxt);

	void initialize_slice(TunnelSliceContext* cntxt, int turn_state, int dx,
			int dy, const Pos& pos);
	void backtrack_slice(Sqr* prev_content, TunnelSliceContext* cntxt, int dep);
	void tunnel_turning_slice(TunnelSliceContext* curr,
			TunnelSliceContext* next);
	void tunnel_straight_slice(TunnelSliceContext* cntxt,
			TunnelSliceContext* next);

	//all return true if valid
	bool validate_slice(Sqr* prev_content, TunnelSliceContext* cntxt, int dep);

private:
	GeneratedLevel& s;
	MTwist& mt;
	int start_room;
	int end_room;
	int padding;
	bool accept_tunnel_entry;
	int avoid_groupid;
	int width;
	int maxdepth;
	int change_odds;
};

/*Sole visible function*/
void generate_tunnels(const TunnelGenSettings& tgs, MTwist& mt,
		GeneratedLevel& level) {

	Pos p;
	bool axis, positive;

	std::vector<Sqr> btbuff;
	std::vector<TunnelSliceContext> tsbuff;

	std::vector<int> genpaths(level.rooms().size(), 0);
	std::vector<int> totalpaths(level.rooms().size(), 0);
	for (int i = 0; i < level.rooms().size(); i++) {
		totalpaths[i] = mt.rand(tgs.min_tunnels, tgs.max_tunnels + 1);
	}
	int nogen_tries = 0;
	while (nogen_tries < 200) {
		nogen_tries++;

		for (int i = 0; i < level.rooms().size(); i++) {
			if (genpaths[i] >= totalpaths[i])
				continue;
			bool generated = false;
			int genwidth = mt.rand(tgs.minwidth, tgs.maxwidth + 1);
			for (; genwidth >= 1 && !generated; genwidth--) {
				int path_len = 5;
				for (int attempts = 0; attempts < 16 && !generated;
						attempts++) {
					TunnelGenImpl tg(
							level,
							mt,
							i + 1,
							tgs.padding,
							genwidth,
							path_len,
							20,
							tgs.padding > 0
									&& (genpaths[i] > 0 || nogen_tries > 100));

					generate_entrance(level.rooms()[i].room_region, mt,
							std::min(genwidth, 2), p, axis, positive);

					int val = positive ? +1 : -1;
					int dx = axis ? 0 : val, dy = axis ? val : 0;

					if (tg.generate(p, dx, dy, btbuff, tsbuff)) {
						genpaths[i]++;
						nogen_tries = 0;
						path_len = 5;
						generated = true;
					}
					if (attempts >= 4) {
						path_len += 5;
					}
				}
			}
		}
	}
}

static Sqr* backtrack_entry(Sqr* backtracking, int entry_size, int entryn) {
	return &backtracking[entry_size * entryn];
}

struct TunnelSliceContext {
	//provided
	Pos p;
	int dx, dy;
	char turn_state;
	int attempt_number;

	//calculated
	bool tunneled;
	Pos ip, newpos;

	TunnelSliceContext() {
	}
};

void TunnelGenImpl::initialize_slice(TunnelSliceContext* cntxt, int turn_state,
		int dx, int dy, const Pos& pos) {
	cntxt->turn_state = turn_state;
	cntxt->dx = dx, cntxt->dy = dy;
	cntxt->p = pos;
	cntxt->ip = Pos(pos.x - (dy != 0 ? 1 : 0), pos.y - (dx != 0 ? 1 : 0));
	cntxt->attempt_number = 0;
}

void TunnelGenImpl::backtrack_slice(Sqr* prev_content,
		TunnelSliceContext* cntxt, int dep) {
	int dx = cntxt->dx, dy = cntxt->dy;

	for (int i = 0; i < width + padding * 2; i++) {
		int xcomp = (dy == 0 ? 0 : i);
		int ycomp = (dx == 0 ? 0 : i);
		s.at(cntxt->ip.x + xcomp, cntxt->ip.y + ycomp) = prev_content[i];
	}

}
bool TunnelGenImpl::validate_slice(Sqr* prev_content, TunnelSliceContext* cntxt,
		int dep) {
	int dx = cntxt->dx, dy = cntxt->dy;
	if (cntxt->p.x <= 2
			|| cntxt->p.x >= s.width() - width - (dx == 0 ? 0 : padding * 2))
		return false;
	if (cntxt->p.y <= 2
			|| cntxt->p.y >= s.height() - width - (dy == 0 ? 0 : padding * 2))
		return false;

	cntxt->tunneled = true;

	for (int i = 0; i < width; i++) {
		int xcomp = (dy == 0 ? 0 : i);
		int ycomp = (dx == 0 ? 0 : i);
		Sqr& sqr = s.at(cntxt->p.x + xcomp, cntxt->p.y + ycomp);
		if (!sqr.passable || (!accept_tunnel_entry && sqr.roomID == 0)
				|| sqr.is_corner) {
			cntxt->tunneled = false;
			break;
		}
		if (avoid_groupid && sqr.groupID == avoid_groupid)
			return false;
		if (sqr.roomID == start_room)
			return false;
	}

	if (cntxt->tunneled) {
		if (cntxt->turn_state != NO_TURN)
			return false;
		return true;
	}

	for (int i = 0; i < width + padding * 2; i++) {
		int xcomp = (dy == 0 ? 0 : i);
		int ycomp = (dx == 0 ? 0 : i);
		Sqr& sqr = s.at(cntxt->ip.x + xcomp, cntxt->ip.y + ycomp);
		if (sqr.passable
				|| (!accept_tunnel_entry && cntxt->turn_state == NO_TURN
						&& sqr.perimeter && sqr.roomID == 0))
			return false;
		memcpy(prev_content + i, &sqr, sizeof(Sqr));
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
			newpos.x = cntxt->p.x + width;
		else
			newpos.x = cntxt->p.x - 1;
		ndx = positive ? +1 : -1;
	} else {
		ndx = 0;
		newpos.x = dx > 0 ? cntxt->p.x - width : cntxt->p.x + 1;
	}

	if (dy == 0) {
		if (positive)
			newpos.y = cntxt->p.y + width;
		else
			newpos.y = cntxt->p.y - 1;
		ndy = positive ? +1 : -1;
	} else {
		ndy = 0;
		newpos.y = dy > 0 ? cntxt->p.y - width : cntxt->p.y + 1;
	}
	return newpos;
}

Pos TunnelGenImpl::next_tunnel_position(TunnelSliceContext* cntxt) {
	Pos newpos;
	int dx = cntxt->dx, dy = cntxt->dy;

	if (dy == 0)
		newpos.y = cntxt->p.y;
	else if (dy > 0)
		newpos.y = cntxt->p.y + 1;
	else
		newpos.y = cntxt->p.y - 1;

	if (dx == 0)
		newpos.x = cntxt->p.x;
	else if (dx > 0)
		newpos.x = cntxt->p.x + 1;
	else
		newpos.x = cntxt->p.x - 1;

	return newpos;
}

void TunnelGenImpl::tunnel_straight_slice(TunnelSliceContext* cntxt,
		TunnelSliceContext* next) {
	int dx = cntxt->dx, dy = cntxt->dy;

	s.at(cntxt->ip).perimeter = true;
	for (int i = 0; i < width; i++) {
		int xcomp = (dy == 0 ? 0 : i);
		int ycomp = (dx == 0 ? 0 : i);

		Sqr& sqr = s.at(cntxt->p.x + xcomp, cntxt->p.y + ycomp);
		sqr.perimeter = false;
		sqr.passable = true;
		sqr.feature = width == 1 ? SMALL_CORRIDOR : LARGE_CORRIDOR;
		sqr.roomID = 0;
	}

	s.at(cntxt->ip.x + (dy == 0 ? 0 : width + 1),
			cntxt->ip.y + (dx == 0 ? 0 : width + 1)).perimeter = true;

	Pos newpos = next_tunnel_position(cntxt);

	bool start_turn = cntxt->turn_state == NO_TURN && mt.rand(change_odds) == 0;
	initialize_slice(next, start_turn ? TURN_START : NO_TURN, dx, dy, newpos);
}
void TunnelGenImpl::tunnel_turning_slice(TunnelSliceContext* cntxt,
		TunnelSliceContext* next) {
	int dx = cntxt->dx, dy = cntxt->dy;
	int ndx, ndy;
	Pos newpos = next_turn_position(cntxt, ndx, ndy);

	for (int i = 0; i < width + padding * 2; i++) {
		int xcomp = (dy == 0 ? 0 : i);
		int ycomp = (dx == 0 ? 0 : i);
		Sqr& sqr = s.at(cntxt->ip.x + xcomp, cntxt->ip.y + ycomp);
		sqr.perimeter = true;
		sqr.passable = false;
		sqr.feature = width == 1 ? SMALL_CORRIDOR : LARGE_CORRIDOR;
		sqr.roomID = 0;
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
bool TunnelGenImpl::generate(Pos p, int dx, int dy, std::vector<Sqr>& btbuff,
		std::vector<TunnelSliceContext>& tsbuff) {

	int entry_size = MAXWIDTH + MAXPADDING * 2;

	__resizebuff(btbuff, entry_size * maxdepth);
	__resizebuff(tsbuff, maxdepth);

	Sqr* backtracking = &btbuff[0];
	TunnelSliceContext* tsc = &tsbuff[0];

	Sqr* prev_content;
	TunnelSliceContext* cntxt;

	bool complete_tunnel = false;
	int tunnel_depth = 0;

	//By setting TURN_PERIMETER we avoid trying a turn on the first tunnel slice
	initialize_slice(&tsc[tunnel_depth], TURN_PERIMETER, dx, dy, p);
	while (true) {
		prev_content = backtrack_entry(backtracking, entry_size, tunnel_depth);
		cntxt = &tsc[tunnel_depth];

		//We must leave room to initialize the next tunnel depth
		bool valid = tunnel_depth < maxdepth - 1
				&& (tsc[tunnel_depth].attempt_number > 0
						|| validate_slice(prev_content, cntxt, tunnel_depth));
		if (valid && cntxt->attempt_number <= 0) {

			if (cntxt->tunneled) {
				end_room = s.at(p).roomID;
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

void generate_entrance(const Region& r, MTwist& mt, int len, Pos& p, bool& axis,
		bool& positive) {
	int ind;
	axis = mt.rand(2), positive = mt.rand(2);
	if (axis) {
		int rmx = r.x + r.w - len;
		if (rmx == r.x + 1)
			ind = rmx;
		else
			ind = mt.rand(r.x + 1, rmx);
		p.y = positive ? r.y + r.h : r.y - 1;
		p.x = ind;
	} else {
		int rmy = r.y + r.h - len;
		if (rmy == r.y + 1)
			ind = rmy;
		else
			ind = mt.rand(r.y + 1, rmy);
		p.x = positive ? r.x + r.w : r.x - 1;
		p.y = ind;
	}
}
