/* Author:  Keith Packard, MIT X Consortium */

/* definitions related to filling of convex polygons, used in code for
 * drawing of wide lines (with caps/joins) via span merging
 */

/* Structure used for moving along left and right edges of polygons via the
   midpoint line algorithm.  The algorithm: increment y, (height-1) times.
   At each step, increment e by dx.  If this makes e positive, also
   subtract dy.  Corresponding change to x: add stepx to x, and (if dy
   needed to be subtracted from e), also add signdx to x. */

typedef struct
{
  unsigned int height;		/* number of scanlines in edge */
  int x;			/* starting x coordinate of edge */
  int stepx;			/* fixed integer dx (usually 0) */
  int signdx;			/* additional (optional) integer dx */
  int e;			/* initial value for decision variable */
  int dy;			/* dy/dx is (rational) slope of edge */
  int dx;
} PolyEdge;

/*
 * types for general polygon routines
 */

typedef struct
{
  double x, y;
} PolyVertex;

typedef struct
{
  int dx, dy;			/* dy/dx is (rational) slope */
  double k;			/* x0 * dy - y0 * dx */
} PolySlope;

/*
 * Line face, used in constructing additional cap/join polygons
 */

typedef struct
{
  double xa, ya;		/* endpoint of line face (rel. to (x,y)) */
  int dx, dy;			/* (dx,dy) points into line (a convention) */
  int x, y;			/* line end, i.e. center of face */
  double k;			/* xa * dy - ya * dx */
} LineFace;


/* Macros for stepping around a convex polygon (i.e. downward from top,
   along the sequence of `left edges' and `right edges') */

/* load fields from next left edge in list */
#define MIPOLYRELOADLEFT    if (!left_height && left_count) { \
	    	    	    	left_height = left->height; \
	    	    	    	left_x = left->x; \
	    	    	    	left_stepx = left->stepx; \
	    	    	    	left_signdx = left->signdx; \
	    	    	    	left_e = left->e; \
	    	    	    	left_dy = left->dy; \
	    	    	    	left_dx = left->dx; \
	    	    	    	--left_count; \
	    	    	    	++left; \
			    }

/* load fields from next right edge in list */
#define MIPOLYRELOADRIGHT   if (!right_height && right_count) { \
	    	    	    	right_height = right->height; \
	    	    	    	right_x = right->x; \
	    	    	    	right_stepx = right->stepx; \
	    	    	    	right_signdx = right->signdx; \
	    	    	    	right_e = right->e; \
	    	    	    	right_dy = right->dy; \
	    	    	    	right_dx = right->dx; \
	    	    	    	--right_count; \
	    	    	    	++right; \
			}

/* Update steps in edge traversal via midpoint line algorithm */

/* step along left edge (modify x appropriately as y is incremented by 1) */
#define MIPOLYSTEPLEFT  left_x += left_stepx; \
    	    	    	left_e += left_dx; \
    	    	    	if (left_e > 0) \
    	    	    	{ \
	    	    	    left_x += left_signdx; \
	    	    	    left_e -= left_dy; \
    	    	    	}

/* step along right edge (modify x appropriately as y is incremented by 1) */
#define MIPOLYSTEPRIGHT right_x += right_stepx; \
    	    	    	right_e += right_dx; \
    	    	    	if (right_e > 0) \
    	    	    	{ \
	    	    	    right_x += right_signdx; \
	    	    	    right_e -= right_dy; \
    	    	    	}
