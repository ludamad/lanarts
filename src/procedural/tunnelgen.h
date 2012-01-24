/*
 * tunnelgen.h
 * DO NOT INCLUDE THIS HEADER!
 * YOU CANNOT INCLUDE THIS HEADER.
 * This file is a part of roomgen.cpp
 */

#ifndef TUNNELGEN_H_
#define TUNNELGEN_H_

//Attempt connection from start room to another room
template<int width, int change_odds = 20>
struct TunnelGen {
	Squares& s;
	MTwist& mt;
	int start_room;
	int end_room;
	bool accept_tunnel_entry;
	int avoid_groupid;
	enum {
		NO_TURN = 0, TURN_PERIMETER = 1, TURN_START = 2,
	};
	inline TunnelGen(Squares& s, MTwist& mt, int start_room, bool ate = false) :
		s(s), mt(mt), start_room(start_room), end_room(0), accept_tunnel_entry(ate),
		avoid_groupid(0){
	}

	bool generate_x(Pos p, bool right, int depth, int turn_state = NO_TURN);
	bool generate_y(Pos p, bool down, int depth, int turn_state = NO_TURN);
};

template<int width, int change_odds>
bool TunnelGen<width, change_odds>::generate_x(Pos p, bool right, int depth,
		int turn_state) {
	bool generated;
	Sqr prev_content[width + 2];//For backtracking
	Pos ip = { p.x, p.y - 1 }, newpos;

	if (p.x <= 2 || p.x >= s.w - width)
		return false;
	if (depth <= 0) return false;
	bool tunneled = true;
	for (int i = 0; i < width; i++) {
		Sqr& sqr = s.at(p.x, p.y + i);
		if (!sqr.passable || (!accept_tunnel_entry && sqr.roomID == 0) ||
				sqr.is_corner){
			tunneled = false;
			break;
		}
		if (avoid_groupid && sqr.groupID == avoid_groupid)
			return false;
		if (sqr.roomID == start_room) return false;
	}
	if (tunneled) {
		if (turn_state != NO_TURN) return false;
		end_room = s.at(p).roomID;
		//s.at(p)
		return true;
	}
	for (int i = 0; i < width + 2; i++) {
		Sqr& sqr = s.at(ip.x, ip.y + i);
		if (sqr.passable || (!accept_tunnel_entry && turn_state == NO_TURN
				&& sqr.perimeter && sqr.roomID == 0))
			return false;
		memcpy(prev_content+i, &sqr, sizeof(Sqr));
	}

	if (turn_state == TURN_START) {
		bool down = rand(mt, 2);
		if (down)
			newpos.y = p.y + width;
		else
			newpos.y = p.y - 1;
		newpos.x = right ? p.x - width : p.x + 1;
		for (int i = 0; i < width + 2; i++) {
			Sqr& sqr = s.at(ip.x, ip.y + i);
			sqr.perimeter = true;
			sqr.passable = false;
			sqr.roomID = 0;
		}
		generated = generate_y(newpos, down, depth - 1, TURN_PERIMETER);
	} else {
		s.at(ip).perimeter = true;
		for (int i = 0; i < width; i++) {
			Sqr& sqr = s.at(p.x, p.y + i);
			sqr.passable = true;
			sqr.perimeter = false;
			sqr.roomID = 0;
		}
		s.at(ip.x, ip.y + width + 1).perimeter = true;

		newpos.y = p.y;
		if (right)
			newpos.x = p.x + 1;
		else
			newpos.x = p.x - 1;

		if (turn_state == NO_TURN && rand(mt, change_odds) == 0) {//95% chance of 90 degree turn
			generated = generate_x(newpos, right, depth - 1, TURN_START);
			if (!generated)
				generated = generate_x(newpos, right, depth - 1);
		} else {
			generated = generate_x(newpos, right, depth - 1);
			if (turn_state == NO_TURN && !generated && rand(mt, change_odds) == 0) {
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
template<int width, int change_odds>
bool TunnelGen<width, change_odds>::generate_y(Pos p, bool down, int depth,
		int turn_state) {
	bool generated;
	Sqr prev_content[width + 2];//For backtracking
	Pos ip = { p.x - 1, p.y }, newpos;

	if (p.y <= 2 || p.y >= s.h - width)
		return false;
	if (depth <= 0) return false;

	bool tunneled = true;
	for (int i = 0; i < width; i++) {
		Sqr& sqr = s.at(p.x + i, p.y);
		if (!sqr.passable || (!accept_tunnel_entry && sqr.roomID == 0)
				|| sqr.is_corner){
			tunneled = false;
			break;
		}
		if (avoid_groupid && sqr.groupID == avoid_groupid)
			return false;
		if (sqr.roomID == start_room) return false;
	}
	if (tunneled) {
		if (turn_state != NO_TURN) return false;
		end_room = s.at(p).roomID;
		return true;
	}
	for (int i = 0; i < width + 2; i++) {
		Sqr& sqr = s.at(ip.x + i, ip.y);
		if (sqr.passable || (!accept_tunnel_entry &&
				turn_state == NO_TURN && sqr.perimeter
				&& sqr.roomID == 0))
			return false;
		memcpy(prev_content+i, &sqr, sizeof(Sqr));
	}

	if (turn_state == TURN_START) {
		bool right = rand(mt, 2);
		if (right)
			newpos.x = p.x + width;
		else
			newpos.x = p.x - 1;
		newpos.y = down ? p.y - width : p.y + 1;
		for (int i = 0; i < width + 2; i++) {
			Sqr& sqr = s.at(ip.x + i, ip.y);
			sqr.perimeter = true;
			sqr.passable = false;
			sqr.roomID = 0;
		}
		generated = generate_x(newpos, right, depth - 1, TURN_PERIMETER);
	} else {
		s.at(ip).perimeter = true;
		for (int i = 0; i < width; i++) {
			Sqr& sqr = s.at(p.x + i, p.y);
			sqr.passable = true;
			sqr.perimeter = false;
			sqr.roomID = 0;
		}
		s.at(ip.x + width + 1, ip.y).perimeter = true;
		newpos.x = p.x;
		if (down)
			newpos.y = p.y + 1;
		else
			newpos.y = p.y - 1;

		if (turn_state == NO_TURN && rand(mt, change_odds) == 0) {//95% chance of 90 degree turn
			generated = generate_y(newpos, down, depth - 1, TURN_START);
			if (!generated)
				generated = generate_y(newpos, down, depth - 1);
		} else {
			generated = generate_y(newpos, down, depth - 1);
			if (turn_state == NO_TURN && !generated && rand(mt, change_odds) == 0) {
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

#endif /* TUNNELGEN_H_ */
