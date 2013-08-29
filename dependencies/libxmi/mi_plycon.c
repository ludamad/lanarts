#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"
#include "mi_scanfill.h"

/* forward references */
static int getPolyYBounds ____P((const miPoint *pts, int n, int *by, int *ty));

/*
 * Written by Brian Kelleher; Dec. 1985.
 * Hacked by Robert S. Maier, 1998-99.
 *
 * Fill a convex polygon (if the polygon is not convex then the result is
 * undefined).  The algorithm is to order the edges from smallest y to
 * largest y, by partitioning the array into a left edge list and a right
 * edge list.  The algorithm used to traverse each edge is an extension of
 * Bresenham's midpoint line algorithm, with y as the major axis.
 *
 * All painting goes through the low-level MI_PAINT_SPANS() macro.
 *
 * See mi_plygen.c for miFillGeneralPoly(), a slower routine that can fill
 * general polygons (i.e. polygons that may be non-convex or
 * self-intersecting).  */

void
#ifdef _HAVE_PROTOS
miFillConvexPoly (miPaintedSet *paintedSet, const miGC *pGC, int count, const miPoint *ptsIn)
#else
miFillConvexPoly (paintedSet, pGC, count, ptsIn)
     miPaintedSet *paintedSet;
     const miGC *pGC;		/* unused */
     int count;
     const miPoint *ptsIn;
#endif
/* count = num of points, ptsIn = the points */
{
  int xl = 0, xr = 0;		/* x vals of left and right edges */
  int dl = 0, dr = 0;		/* decision variables             */
  int ml = 0, m1l = 0;		/* left edge slope and slope+1    */
  int mr = 0, m1r = 0;		/* right edge slope and slope+1   */
  int incr1l = 0, incr2l = 0;	/* left edge error increments     */
  int incr1r = 0, incr2r = 0;	/* right edge error increments    */
  int dy;			/* delta y                        */
  int y;			/* current scanline               */
  int left, right;		/* indices to first endpoints     */
  int i;			/* loop counter                   */
  int nextleft, nextright;	/* indices to second endpoints    */
  miPoint *ptsOut, *FirstPoint; /* output buffer                  */
  unsigned int *width, *FirstWidth; /* output buffer                  */
  int imin;			/* index of smallest vertex (in y) */
  int ymin;			/* y-extents of polygon            */
  int ymax;
  
  /*
   *  find leftx, bottomy, rightx, topy, and the index
   *  of bottomy. Also translate the points.
   */
  imin = getPolyYBounds(ptsIn, count, &ymin, &ymax);
  
  dy = ymax - ymin + 1;
  if ((count < 3) || (dy < 0))
    return;
  ptsOut = FirstPoint = (miPoint *)mi_xmalloc(sizeof(miPoint) * dy);
  width = FirstWidth = (unsigned int *)mi_xmalloc(sizeof(unsigned int) * dy);
  
  nextleft = nextright = imin;
  y = ptsIn[nextleft].y;
  
  /*
   *  loop through all edges of the polygon
   */
  do {
    /*
     *  add a left edge if we need to
     */
    if (ptsIn[nextleft].y == y) 
      {
	left = nextleft;

	/*
	 *  find the next edge, considering the end
	 *  conditions of the array.
	 */
	nextleft++;
	if (nextleft >= count)
	  nextleft = 0;

	/*
	 *  now compute all of the random information
	 *  needed to run the iterative algorithm.
	 */
	BRESINITPGON(ptsIn[nextleft].y-ptsIn[left].y,
		     ptsIn[left].x,ptsIn[nextleft].x,
		     xl, dl, ml, m1l, incr1l, incr2l);
      }
    
    /*
     *  add a right edge if we need to
     */
    if (ptsIn[nextright].y == y) 
      {
	right = nextright;

	/*
	 *  find the next edge, considering the end
	 *  conditions of the array.
	 */
	nextright--;
	if (nextright < 0)
	  nextright = count-1;

	/*
	 *  now compute all of the random information
	 *  needed to run the iterative algorithm.
	 */
	BRESINITPGON(ptsIn[nextright].y-ptsIn[right].y,
		     ptsIn[right].x,ptsIn[nextright].x,
		     xr, dr, mr, m1r, incr1r, incr2r);
      }
    
    /*
     *  generate scans to fill while we still have
     *  a right edge as well as a left edge.
     */
    i = IMIN(ptsIn[nextleft].y, ptsIn[nextright].y) - y;
    /* in case we're called with non-convex polygon */
    if(i < 0)
      {
	free (FirstWidth);
	free (FirstPoint);
	return;
      }

    while (i-- > 0) 
      {
	ptsOut->y = y;

	/*
	 *  reverse the edges if necessary
	 */
	if (xl < xr) 
	  {
	    *(width++) = (unsigned int)(xr - xl);
	    (ptsOut++)->x = xl;
	  }
	else 
	  {
	    *(width++) = (unsigned int)(xl - xr);
	    (ptsOut++)->x = xr;
	  }
	y++;

	/* increment down the edges */
	BRESINCRPGON(dl, xl, ml, m1l, incr1l, incr2l);
	BRESINCRPGON(dr, xr, mr, m1r, incr1r, incr2r);
      }
  }  while (y != ymax);
  
  /*
   * Finally, paint the <remaining> spans
   */
  MI_PAINT_SPANS(paintedSet, pGC->pixels[1], ptsOut - FirstPoint, FirstPoint, FirstWidth)
}

/*
 *     Find the index of the point with the smallest y.
 */
static int
#ifdef _HAVE_PROTOS
getPolyYBounds (const miPoint *pts, int n, int *by, int *ty)
#else
getPolyYBounds (pts, n, by, ty)
     const miPoint *pts;
     int n;
     int *by, *ty;
#endif
{
  const miPoint *ptsStart = pts;
  const miPoint *ptMin;
  int ymin, ymax;

  ptMin = pts;
  ymin = ymax = (pts++)->y;

  while (--n > 0) 
    {
      if (pts->y < ymin)
	{
	  ptMin = pts;
	  ymin = pts->y;
        }
      if(pts->y > ymax)
	ymax = pts->y;

      pts++;
    }

  *by = ymin;
  *ty = ymax;
  return (ptMin - ptsStart);
}
