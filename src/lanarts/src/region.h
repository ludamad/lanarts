/*
 * region.h:
 * 	XXX get rid of this structure in favour of bbox everywhere ?
 */

#ifndef REGION_H_
#define REGION_H_

#include <lcommon/geometry.h>


//TODO replace region with BBox everywhere
/*Represents a rectangular region in terms of its start x,y values as well as its dimensions*/
struct Region {
	int x, y, w, h;
	Region(int x = 0, int y = 0, int w = 0, int h = 0) :
			x(x), y(y), w(w), h(h) {
	}
	inline Pos pos() {
		return Pos(x, y);
	}
	Region remove_perimeter(int p = 1) {
		return Region(x + p, y + p, w - 2 * p, h - 2 * p);
	}

};


#endif /* REGION_H_ */
