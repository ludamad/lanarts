/*
 * tunnelgen.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#include "tunnelgen.h"
#include "mtwist.h"

//Attempt connection from start room to another room
struct TunnelGen {
	GeneratedLevel& s;
	MTwist& mt;
	int start_room;
	int end_room;
	bool accept_tunnel_entry;
	int avoid_groupid;
	int width;
	int change_odds;
	enum {
		NO_TURN = 0, TURN_PERIMETER = 1, TURN_START = 2,
	};
	inline TunnelGen(GeneratedLevel& s, MTwist& mt, int start_room, int width,
			int change_odds, bool ate = false) :
			s(s), mt(mt), start_room(start_room), end_room(0), accept_tunnel_entry(
					ate), avoid_groupid(0), width(width), change_odds(
					change_odds) {
	}

	bool generate_x(Pos p, bool right, int depth, int turn_state = NO_TURN);
	bool generate_y(Pos p, bool down, int depth, int turn_state = NO_TURN);
};

bool TunnelGen::generate_x(Pos p, bool right, int depth, int turn_state) {
	bool generated;
	Sqr prev_content[width + 2]; //For backtracking
	Pos ip(p.x, p.y - 1), newpos;

	if (p.x <= 2 || p.x >= s.width() - width)
		return false;
	if (depth <= 0)
		return false;
	bool tunneled = true;
	for (int i = 0; i < width; i++) {
		Sqr& sqr = s.at(p.x, p.y + i);
		if (!sqr.passable || (!accept_tunnel_entry && sqr.roomID == 0)
				|| sqr.is_corner) {
			tunneled = false;
			break;
		}
		if (avoid_groupid && sqr.groupID == avoid_groupid)
			return false;
		if (sqr.roomID == start_room)
			return false;
	}
	if (tunneled) {
		if (turn_state != NO_TURN)
			return false;
		end_room = s.at(p).roomID;
		//s.at(p)
		return true;
	}
	for (int i = 0; i < width + 2; i++) {
		Sqr& sqr = s.at(ip.x, ip.y + i);
		if (sqr.passable
				|| (!accept_tunnel_entry && turn_state == NO_TURN
						&& sqr.perimeter && sqr.roomID == 0))
			return false;
		memcpy(prev_content + i, &sqr, sizeof(Sqr));
	}

	if (turn_state == TURN_START) {
		bool down = mt.rand(2);
		if (down)
			newpos.y = p.y + width;
		else
			newpos.y = p.y - 1;
		newpos.x = right ? p.x - width : p.x + 1;
		for (int i = 0; i < width + 2; i++) {
			Sqr& sqr = s.at(ip.x, ip.y + i);
			sqr.perimeter = true;
			sqr.passable = false;
			sqr.feature = width == 1 ? SMALL_CORRIDOR : LARGE_CORRIDOR;
			sqr.roomID = 0;
		}
		generated = generate_y(newpos, down, depth - 1, TURN_PERIMETER);
	} else {
		s.at(ip).perimeter = true;
		for (int i = 0; i < width; i++) {
			Sqr& sqr = s.at(p.x, p.y + i);
			sqr.perimeter = false;
			sqr.passable = true;
			sqr.feature = width == 1 ? SMALL_CORRIDOR : LARGE_CORRIDOR;
			sqr.roomID = 0;
		}
		s.at(ip.x, ip.y + width + 1).perimeter = true;

		newpos.y = p.y;
		if (right)
			newpos.x = p.x + 1;
		else
			newpos.x = p.x - 1;

		if (turn_state == NO_TURN && mt.rand(change_odds) == 0) { //95% chance of 90 degree turn
			generated = generate_x(newpos, right, depth - 1, TURN_START);
			if (!generated)
				generated = generate_x(newpos, right, depth - 1);
		} else {
			generated = generate_x(newpos, right, depth - 1);
			if (turn_state == NO_TURN && !generated
					&& mt.rand(change_odds) == 0) {
				generated = generate_x(newpos, right, depth - 1, TURN_START);
			}
		}
	}
	if (!generated) {
		for (int i = 0; i < width + 2; i++) {
			s.at(ip.x, ip.y + i) = prev_content[i];
		}
	}
	return generated;
}
bool TunnelGen::generate_y(Pos p, bool down, int depth, int turn_state) {
	bool generated;
	Sqr prev_content[width + 2]; //For backtracking
	Pos ip(p.x - 1, p.y), newpos;

	if (p.y <= 2 || p.y >= s.height() - width)
		return false;
	if (depth <= 0)
		return false;

	bool tunneled = true;
	for (int i = 0; i < width; i++) {
		Sqr& sqr = s.at(p.x + i, p.y);
		if (!sqr.passable || (!accept_tunnel_entry && sqr.roomID == 0)
				|| sqr.is_corner) {
			tunneled = false;
			break;
		}
		if (avoid_groupid && sqr.groupID == avoid_groupid)
			return false;
		if (sqr.roomID == start_room)
			return false;
	}
	if (tunneled) {
		if (turn_state != NO_TURN)
			return false;
		end_room = s.at(p).roomID;
		return true;
	}
	for (int i = 0; i < width + 2; i++) {
		Sqr& sqr = s.at(ip.x + i, ip.y);
		if (sqr.passable
				|| (!accept_tunnel_entry && turn_state == NO_TURN
						&& sqr.perimeter && sqr.roomID == 0))
			return false;
		memcpy(prev_content + i, &sqr, sizeof(Sqr));
	}

	if (turn_state == TURN_START) {
		bool right = mt.rand(2);
		if (right)
			newpos.x = p.x + width;
		else
			newpos.x = p.x - 1;
		newpos.y = down ? p.y - width : p.y + 1;
		for (int i = 0; i < width + 2; i++) {
			Sqr& sqr = s.at(ip.x + i, ip.y);
			sqr.perimeter = true;
			sqr.passable = false;
			sqr.feature = width == 1 ? SMALL_CORRIDOR : LARGE_CORRIDOR;
			sqr.roomID = 0;
		}
		generated = generate_x(newpos, right, depth - 1, TURN_PERIMETER);
	} else {
		s.at(ip).perimeter = true;
		for (int i = 0; i < width; i++) {
			Sqr& sqr = s.at(p.x + i, p.y);
			sqr.perimeter = false;
			sqr.passable = true;
			sqr.feature = width == 1 ? SMALL_CORRIDOR : LARGE_CORRIDOR;
			sqr.roomID = 0;
		}
		s.at(ip.x + width + 1, ip.y).perimeter = true;
		newpos.x = p.x;
		if (down)
			newpos.y = p.y + 1;
		else
			newpos.y = p.y - 1;

		if (turn_state == NO_TURN && mt.rand(change_odds) == 0) { //95% chance of 90 degree turn
			generated = generate_y(newpos, down, depth - 1, TURN_START);
			if (!generated)
				generated = generate_y(newpos, down, depth - 1);
		} else {
			generated = generate_y(newpos, down, depth - 1);
			if (turn_state == NO_TURN && !generated
					&& mt.rand(change_odds) == 0) {
				generated = generate_y(newpos, down, depth - 1, TURN_START);
			}
		}
	}
	if (!generated) {
		for (int i = 0; i < width + 2; i++) {
			s.at(ip.x + i, ip.y) = prev_content[i];
		}
	}
	return generated;
}

void generate_entrance(const Region& r, MTwist& mt, int len, Pos& p, bool& axis,
		bool& more) {
	int ind;
	axis = mt.rand(2), more = mt.rand(2);
	if (axis) {
		int rmx = r.x + r.w - len;
		ind = mt.rand(r.x + 1, rmx);
		p.y = more ? r.y + r.h : r.y - 1;
		p.x = ind;
	} else {
		int rmy = r.y + r.h - len;
		ind = mt.rand(r.y + 1, rmy);
		p.x = more ? r.x + r.w : r.x - 1;
		p.y = ind;
	}
}

//TODO: Refactor generate_x and generate_y as one templated function!
void generate_tunnels(const TunnelGenSettings& tgs, MTwist& mt,
		GeneratedLevel& level) {

	Pos p;
	bool axis, more;
	std::vector<int> genpaths(level.rooms().size(), 0);
	std::vector<int> totalpaths(level.rooms().size());
	for (int i = 0; i < level.rooms().size(); i++) {
		totalpaths[i] = mt.rand(tgs.min_tunnels, tgs.max_tunnels+1);
	}

	int nogen_tries = 0;
	while (nogen_tries < 200) {
		nogen_tries++;

		for (int i = 0; i < level.rooms().size(); i++) {
			if (genpaths[i] >= totalpaths[i])
				continue;
			TunnelGen tg(level, mt, i + 1,
					mt.rand(tgs.minwidth, tgs.maxwidth + 1), 20,
					genpaths[i] > 0);
			bool generated = false;
			for (; tg.width >= tgs.minwidth && !generated; tg.width--) {
				int path_len = 5;
				for (int attempts = 0; attempts < 16 && !generated;
						attempts++) {
					//bool small = (havepath && mt.rand(2) == 0);
					generate_entrance(level.rooms()[i].room_region, mt, 2, p,
							axis, more);
					if (axis) {
						if (tg.generate_y(p, more, path_len)) {
							genpaths[i]++;
							nogen_tries = 0;
							path_len = 5;
							generated = true;
						}
					} else {
						if (tg.generate_x(p, more, path_len)) {
								genpaths[i]++;
								nogen_tries = 0;
								path_len = 5;
								generated = true;
						}
					}
						if (attempts >= 4) {
							path_len += 5;
						}
					}
				}
				//tg.end_room
			}
		}
	}
