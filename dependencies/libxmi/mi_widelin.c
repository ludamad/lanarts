#include "sys-defines.h"
#include "extern.h"

/* Original author: Keith Packard, MIT X Consortium.  
   Hacked by Robert S. Maier, 1998-99. */

/* This module contains the miWideLine() and miWideDash() functions.  They
   rasterize wide polylines (usually just called `wide lines'), either
   solid or dashed.
   
   Any wide line is treated as a polygon to be painted.  (If it is dashed,
   each dash is treated as a polygon.)  The painting follows libxmi's
   policy on painting of `edge' pixels, i.e., pixels that lie exactly on a
   boundary.  A pixel is not painted if it lies on a `right' or `bottom'
   edge of a polygon.

   All painting goes through the low-level MI_PAINT_SPANS() macro. */

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"
#include "mi_widelin.h"

/* undefine if hypot is available (it's X_OPEN, but not ANSI or POSIX) */
#define hypot(x, y) sqrt((x)*(x) + (y)*(y))

/* internal functions that do painting of pixels */
static void miFillPolyHelper ____P((miPaintedSet *paintedSet, miPixel pixel, int y, unsigned int overall_height, PolyEdge *left, PolyEdge *right, int left_count, int right_count));
static void miFillRectPolyHelper ____P((miPaintedSet *paintedSet, miPixel pixel, int x, int y, unsigned int w, unsigned int h));
static void miLineArc ____P((miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, LineFace *leftFace, LineFace *rightFace, double xorg, double yorg, bool isInt));
static void miLineJoin ____P((miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, LineFace *pLeft, LineFace *pRight));
static void miLineProjectingCap ____P((miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, const LineFace *face, bool isLeft, bool isInt));
static void miWideDashSegment ____P((miPaintedSet *paintedSet, const miGC *pGC, int *pDashNum, int *pDashIndex, int *pDashOffset, int x1, int y1, int x2, int y2, bool projectLeft, bool projectRight, LineFace *leftFace, LineFace *rightFace));
static void miWideSegment ____P((miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, int x1, int y1, int x2, int y2, bool projectLeft, bool projectRight, LineFace *leftFace, LineFace *rightFace));

/* internal functions that don't do painting of pixels */
static int miLineArcD ____P((const miGC *pGC, double xorg, double yorg, miPoint *points, unsigned int *widths, PolyEdge *edge1, int edgey1, bool edgeleft1, PolyEdge *edge2, int edgey2, bool edgeleft2));
static int miLineArcI ____P((const miGC *pGC, int xorg, int yorg, miPoint *points, unsigned int *widths));
static int miPolyBuildEdge ____P((double x0, double y0, double k, int dx, int dy, int xi, int yi, bool left, PolyEdge *edge));
static int miPolyBuildPoly ____P((const PolyVertex *vertices, const PolySlope *slopes, int count, int xi, int yi, PolyEdge *left, PolyEdge *right, int *pnleft, int *pnright, unsigned int *h));
static int miRoundCapClip ____P((const LineFace *face, bool isInt, PolyEdge *edge, bool *leftEdge));
static int miRoundJoinFace ____P((const LineFace *face, PolyEdge *edge, bool *leftEdge));
static void miRoundJoinClip ____P((LineFace *pLeft, LineFace *pRight, PolyEdge *edge1, PolyEdge *edge2, int *y1, int *y2, bool *left1, bool *left2));

/* Spans-based convex polygon filler.  Paints a convex polygon, supplied as
   lists of `left' and `right' edges.  Used for painting polygonal line
   caps and line joins.

   This implements libxmi's policy on painting `edge' pixels, i.e., pixels
   that lie exactly on the boundary of a polygon.  A pixel is not painted
   if it lies on a `right' or `bottom' edge of the polygon. */

static void
#ifdef _HAVE_PROTOS
miFillPolyHelper (miPaintedSet *paintedSet, miPixel pixel, int y, unsigned int overall_height, PolyEdge *left, PolyEdge *right, int left_count, int right_count)
#else
miFillPolyHelper (paintedSet, pixel, y, overall_height, left, right, left_count, right_count)
     miPaintedSet *paintedSet;
     miPixel pixel;
     int y;			/* starting y coordinate */
     unsigned int overall_height; /* height of entire segment */
     PolyEdge *left, *right;
     int left_count, right_count;
#endif
{
  int 	left_x = 0, left_e = 0;
  int	left_stepx = 0;
  int	left_signdx = 0;
  int	left_dy = 0, left_dx = 0;

  int 	right_x = 0, right_e = 0;
  int	right_stepx = 0;
  int	right_signdx = 0;
  int	right_dy = 0, right_dx = 0;

  unsigned int	left_height = 0, right_height = 0;

  miPoint 	*ppt;
  miPoint 	*pptInit = (miPoint *)NULL;
  unsigned int 	*pwidth;
  unsigned int 	*pwidthInit = (unsigned int *)NULL;

  pptInit = (miPoint *)mi_xmalloc(overall_height * sizeof(miPoint));
  pwidthInit = (unsigned int *)mi_xmalloc(overall_height * sizeof(unsigned int));
  ppt = pptInit;
  pwidth = pwidthInit;

  while ((left_count || left_height) && (right_count || right_height))
    {
      unsigned int height;

      /* load fields from next left edge, right edge */
      MIPOLYRELOADLEFT
      MIPOLYRELOADRIGHT

      height = UMIN (left_height, right_height);
      left_height -= height;
      right_height -= height;
      /* walk down to end of left or right edge, whichever comes first */
      while (height--)
	{
	  if (right_x >= left_x)
	    /* generate a span (omitting point on right end, see above) */
	    {
	      ppt->x = left_x;
	      ppt->y = y;
	      ppt++;
	      *pwidth++ = (unsigned int)(right_x - left_x + 1);
	    }
	  y++;
    	
	  /* update left_x, right_x by stepping along left and right edges,
	     using midpoint line algorithm */
	  MIPOLYSTEPLEFT
	  MIPOLYSTEPRIGHT
	}
    }

  MI_PAINT_SPANS(paintedSet, pixel, ppt - pptInit, pptInit, pwidthInit)
}

/* Rectangle filler.  Policy mentioned above (no painting of right or
   bottom edges) is followed. */

static void
#ifdef _HAVE_PROTOS
miFillRectPolyHelper (miPaintedSet *paintedSet, miPixel pixel, int x, int y, unsigned int w, unsigned int h)
#else
miFillRectPolyHelper (paintedSet, pixel, x, y, w, h)
     miPaintedSet *paintedSet;
     miPixel pixel;
     int x, y;
     unsigned int w, h;
#endif
{
  miPoint *ppt, *pptInit;
  unsigned int *pwidth, *pwidthInit;

  pptInit = (miPoint *)mi_xmalloc (h * sizeof(miPoint));
  pwidthInit = (unsigned int *)mi_xmalloc (h * sizeof(unsigned int));
  ppt = pptInit;
  pwidth = pwidthInit;
  while (h--)
    {
      *pwidth++ = w;
      ppt->x = x;
      ppt->y = y;
      ppt++;
      y++;
    }

  MI_PAINT_SPANS(paintedSet, pixel, ppt - pptInit, pptInit, pwidthInit)
}

/* Build a single polygon edge (either a left edge or a right edge).
   I.e. compute integer edge data that can be used by the midpoint line
   algorithm.  The edge will be traversed downward (on entry, dy != 0 is
   assumed).  Returns starting value for y, i.e. integer y-value for top of
   edge. */

/* Supplied: an integer offset (xi,yi), the exact floating-point edge start
   (x0,y0), its (rational) slope dy/dx, and the quantity k = x0*dy-y0*dx,
   which is a measure of distance of (x0,y0) from the parallel line segment
   that passes through (0,0).  k will be transformed into the initial value
   for e, the integer decision variable for the midpoint line algorithm. */

/* The integer edge data that are computed do not include the `height'
   field, i.e. the number of scanlines to process. */

static int
#ifdef _HAVE_PROTOS
miPolyBuildEdge (double x0, double y0, double k, int dx, int dy, int xi, int yi, bool left, PolyEdge *edge)
#else
miPolyBuildEdge (x0, y0, k, dx, dy, xi, yi, left, edge)
     double x0, y0;		/* starting point of edge (rel. to (xi,yi)) */
     double k;			/* x0 * dy - y0 * dx */
     int dx, dy;		/* edge has rational slope dy/dx */
     int xi, yi;		/* integer offset for coordinate system */
     bool left;			/* left edge, not right edge? */
     PolyEdge *edge;		/* integer edge data, to be filled in */
#endif
{
  int x, y, e;
  int xady;

  /* make dy positive, since edge will be traversed downward */
  if (dy < 0)
    {
      dy = -dy;
      dx = -dx;
      k = -k;
    }

#if 0
  {
    double realk, kerror;

    realk = x0 * dy - y0 * dx;
    kerror = fabs (realk - k);
    if (kerror > .1)
      printf ("realk: %g\t k: %g\n", realk, k);
  }
#endif

  /* integer starting value for y: round up the floating-point value */
  y = ICEIL (y0);

  /* work out integer starting value for x */
  xady = ICEIL (k) + y * dx;
  if (xady <= 0)
    x = - (-xady / dy) - 1;
  else
    x = (xady - 1) / dy;

  /* start working out initial value of decision variable */
  e = xady - x * dy;		/* i.e. ICEIL(k) - (x * dy - y * dx) */

  /* work out optional and non-optional x increment, for algorithm */
  if (dx >= 0)
    {
      edge->signdx = 1;		/* optional step */
      edge->stepx = dx / dy;	/* non-optional step, 0 if dx<dy in mag. */
      edge->dx = dx % dy;
    }
  else
    {
      edge->signdx = -1;	/* optional step */
      edge->stepx = - (-dx / dy); /* non-optional step, 0 if dx<dy in mag. */
      edge->dx = -dx % dy;
      e = dy - e + 1;
    }
  edge->dy = dy;
  edge->x = x + (left == true ? 1 : 0) + xi; /* starting value for x */
  edge->e = e - dy;		/* bias: initial value for e */

  /* return integer starting value for y, i.e. top of edge */
  return y + yi;
}

/* add incr to v, cyclically; always stay in range 0..max */
#define StepAround(v, incr, max) (((v) + (incr) < 0) ? (max - 1) : ((v) + (incr) == max) ? 0 : ((v) + (incr)))

/* Build lists of right and left polygon edges, from an array of vertex
   coordinates (floating-point), a precomputed array of PolySlopes
   (including a `k' value for each edge), and an integer offset vector.
   Also return overall vertical range and top (starting) y value. */

static int
#ifdef _HAVE_PROTOS
miPolyBuildPoly (const PolyVertex *vertices, const PolySlope *slopes, int count, int xi, int yi, PolyEdge *left, PolyEdge *right, int *pnleft, int *pnright, unsigned int *h)
#else
miPolyBuildPoly (vertices, slopes, count, xi, yi, left, right, pnleft, pnright, h)
     const PolyVertex *vertices;
     const PolySlope *slopes;
     int count;
     int xi, yi;		/* (xi,yi) = integer offset for polygon */
     PolyEdge *left, *right;
     int *pnleft, *pnright;
     unsigned int *h;
#endif
{
  int	    top, bottom;
  double    miny, maxy;
  int       i;
  int       j;
  int	    clockwise;
  int	    slopeoff;
  int       s;
  int       nright, nleft;
  int       y, lasty = 0, bottomy, topy = 0;

  /* compute min, max y values for polygon (floating-point); also location
     of corresponding vertices in vertex array */
  maxy = miny = vertices[0].y;
  bottom = top = 0;
  for (i = 1; i < count; i++)
    {
      if (vertices[i].y < miny)
	{
	  top = i;
	  miny = vertices[i].y;
	}
      if (vertices[i].y >= maxy)
	{
	  bottom = i;
	  maxy = vertices[i].y;
	}
    }

  /* compute integer y-value for bottom of polygon (round up) */
  bottomy = ICEIL (maxy) + yi;

  /* determine whether should go `clockwise' or `counterclockwise'
     to move down the right side of the polygon */
  i = top;
  j = StepAround (top, -1, count);
  clockwise = 1;
  slopeoff = 0;
  if (slopes[j].dy * slopes[i].dx > slopes[i].dy * slopes[j].dx)
    {
      clockwise = -1;
      slopeoff = -1;
    }

  /* step around right side of polygon from top to bottom, building array
     of `right' edges (horizontal edges are ignored) */
  i = top;
  s = StepAround (top, slopeoff, count);
  nright = 0;
  while (i != bottom)
    {
      if (slopes[s].dy != 0)
	{
	  y = miPolyBuildEdge (vertices[i].x, vertices[i].y,
			       slopes[s].k, slopes[s].dx, slopes[s].dy,
			       xi, yi, false,
			       &right[nright]);
	  if (nright != 0)
	    right[nright-1].height = y - lasty;
	  else			/* y is top of first edge */
	    topy = y;
	  nright++;
	  lasty = y;
	}

      i = StepAround (i, clockwise, count);
      s = StepAround (s, clockwise, count);
    }
  if (nright != 0)
    right[nright-1].height = bottomy - lasty;

  /* step around left side of polygon from top to bottom, building array of
     `left' edges (horizontal edges are ignored) */
  if (slopeoff == 0)
    slopeoff = -1;
  else
    slopeoff = 0;
  i = top;
  s = StepAround (top, slopeoff, count);
  nleft = 0;
  while (i != bottom)
    {
      if (slopes[s].dy != 0)
	{
	  y = miPolyBuildEdge (vertices[i].x, vertices[i].y,
			       slopes[s].k, slopes[s].dx,  slopes[s].dy,
			       xi, yi, true,
			       &left[nleft]);
    
	  if (nleft != 0)
	    left[nleft-1].height = y - lasty;
	  nleft++;
	  lasty = y;
	}

      i = StepAround (i, -clockwise, count);
      s = StepAround (s, -clockwise, count);
    }
  if (nleft != 0)
    left[nleft-1].height = bottomy - lasty;

  /* return number of left-side and right-side edges; also height (vertical
     range, an unsigned int) and the vertical location of the top vertex
     (an integer) */
  *pnleft = nleft;
  *pnright = nright;
  *h = bottomy - topy;

  return topy;
}

/* Paint all types of line join: round/miter/bevel/triangular.  Called by
   both miWideLine() and miWideDash().  Left and right line faces are
   supplied, each with its own value of k.  They may be modified. */
static void
#ifdef _HAVE_PROTOS
miLineJoin (miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, LineFace *pLeft, LineFace *pRight)
#else
miLineJoin (paintedSet, pixel, pGC, pLeft, pRight)
     miPaintedSet *paintedSet;
     miPixel pixel;
     const miGC *pGC;
     LineFace *pLeft, *pRight;
#endif
{
  double	    mx = 0.0, my = 0.0;
  int		    denom = 0;	/* avoid compiler warnings */
  PolyVertex   	    vertices[4];
  PolySlope    	    slopes[4];
  int		    edgecount;
  PolyEdge          left[4], right[4];
  int               nleft, nright;
  int               y;
  unsigned int      height;
  bool		    swapslopes;
  int		    joinStyle = (int)pGC->joinStyle;
  int		    lw = (int)(pGC->lineWidth);

  if (joinStyle == (int)MI_JOIN_ROUND)
    {
      /* invoke miLineArc to fill the round join, isInt = true */
      miLineArc (paintedSet, pixel, pGC,
		 pLeft, pRight, (double)0.0, (double)0.0, true);
      return;
    }

  denom = - pLeft->dx * pRight->dy + pRight->dx * pLeft->dy;
  if (denom == 0)
    return;			/* no join to draw */

  /* Now must handle cases where line join is a small polygon to be filled;
     specify its vertices clockwise. */

  /* swap slopes if cross product of line faces has wrong sign */
  if (denom > 0)
    {
      swapslopes = false;
      pLeft->xa = -pLeft->xa;
      pLeft->ya = -pLeft->ya;
      pLeft->dx = -pLeft->dx;
      pLeft->dy = -pLeft->dy;
    }
  else
    {
      swapslopes = true;
      pRight->xa = -pRight->xa;
      pRight->ya = -pRight->ya;
      pRight->dx = -pRight->dx;
      pRight->dy = -pRight->dy;
    }

  /* vertex #0 is at the right end of the right face */
  vertices[0].x = pRight->xa;
  vertices[0].y = pRight->ya;
  slopes[0].dx = -pRight->dy;
  slopes[0].dy =  pRight->dx;
  slopes[0].k = 0;

  /* vertex #1 is the nominal join point (i.e. halfway across both the
     right face and the left face) */
  vertices[1].x = 0;
  vertices[1].y = 0;
  slopes[1].dx =  pLeft->dy;
  slopes[1].dy = -pLeft->dx;
  slopes[1].k = 0;

  /* vertex #2 is at the left end of the left face */
  vertices[2].x = pLeft->xa;
  vertices[2].y = pLeft->ya;

  if (joinStyle == (int)MI_JOIN_MITER)
    {
      double miterlimit = pGC->miterLimit;

      /* compute vertex (mx,my) of miter quadrilateral */
      my = (pLeft->dy  * (pRight->xa * pRight->dy - pRight->ya * pRight->dx) -
	    pRight->dy * (pLeft->xa  * pLeft->dy  - pLeft->ya  * pLeft->dx )) /
	      (double) denom;
      if (pLeft->dy != 0)
	mx = pLeft->xa + (my - pLeft->ya) *
	  (double) pLeft->dx / (double) pLeft->dy;
      else
	mx = pRight->xa + (my - pRight->ya) *
	  (double) pRight->dx / (double) pRight->dy;
      /* if miter limit violated, switch to bevelled join */
      if ((mx * mx + my * my) * 4 > miterlimit * miterlimit * lw * lw)
	joinStyle = (int)MI_JOIN_BEVEL;
    }

  switch ((int)joinStyle)
    {
      double scale, dx, dy, adx, ady;

    case (int)MI_JOIN_MITER:
    default:
      /* join by adding a quadrilateral */
      edgecount = 4;

      slopes[2].dx = pLeft->dx;
      slopes[2].dy = pLeft->dy;
      slopes[2].k =  pLeft->k;
      if (swapslopes)
	{
	  slopes[2].dx = -slopes[2].dx;
	  slopes[2].dy = -slopes[2].dy;
	  slopes[2].k  = -slopes[2].k;
	}

      /* vertex #3 is miter vertex (mx,my) */
      vertices[3].x = mx;
      vertices[3].y = my;
      slopes[3].dx = pRight->dx;
      slopes[3].dy = pRight->dy;
      slopes[3].k  = pRight->k;
      if (swapslopes)
	{
	  slopes[3].dx = -slopes[3].dx;
	  slopes[3].dy = -slopes[3].dy;
	  slopes[3].k  = -slopes[3].k;
	}
      break;

    case (int)MI_JOIN_BEVEL:
      /* join by adding a triangle */
      {
	PolyVertex midpoint;

	edgecount = 3;
	
	/* third edge of triangle will pass through midpoint */
	midpoint.x = 0.5 * (pLeft->xa + pRight->xa);
	midpoint.y = 0.5 * (pLeft->ya + pRight->ya);

	/* vector along third edge of triangle */
	dx = pRight->xa - pLeft->xa;
	dy = pRight->ya - pLeft->ya;

	/* compute scale = max(|dx|,|dy|) */
	adx = dx;
	ady = dy;
	if (adx < 0)
	  adx = -adx;
	if (ady < 0)
	  ady = -ady;
	scale = ady;
	if (adx > ady)
	  scale = adx;
	
	/* use integer dx, dy in range -65536..65536 */
	slopes[2].dx = (int)((dx * 65536) / scale);
	slopes[2].dy = (int)((dy * 65536) / scale);
	slopes[2].k = midpoint.x * slopes[2].dy - midpoint.y * slopes[2].dx;
      }
      break;

    case (int)MI_JOIN_TRIANGULAR:
      /* join by adding a stubby quadrilateral */
      {
	PolyVertex midpoint, newpoint;
	double mid2, mid, dx2, dy2, dx3, dy3;
	
	edgecount = 4;
	
	/* compute additional vertex, offset by linewidth/2 */
	midpoint.x = 0.5 * (pLeft->xa + pRight->xa);
	midpoint.y = 0.5 * (pLeft->ya + pRight->ya);
	mid2 = midpoint.x * midpoint.x + midpoint.y * midpoint.y;
	mid = sqrt (mid2);
	newpoint.x = 0.5 * lw * midpoint.x / mid;
	newpoint.y = 0.5 * lw * midpoint.y / mid;	
	vertices[3] = newpoint;

	/* offset from vertices[2] to vertices[3] */
	dx2 = vertices[3].x - vertices[2].x;
	dy2 = vertices[3].y - vertices[2].y;	

	/* offset from vertices[3] back to vertices[0] */
	dx3 = vertices[0].x - vertices[3].x;
	dy3 = vertices[0].y - vertices[3].y;	

	/* compute scale = max(|dx|,|dy|), where (dx,dy) is offset between
	   the two corners, i.e. vertices[0] and vertices[2] */
	dx = pRight->xa - pLeft->xa;
	dy = pRight->ya - pLeft->ya;
	adx = dx;
	ady = dy;
	if (adx < 0)
	  adx = -adx;
	if (ady < 0)
	  ady = -ady;
	scale = ady;
	if (adx > ady)
	  scale = adx;
	
	/* use integer dx, dy in range -65536..65536 */
	slopes[2].dx = (int)((dx2 * 65536) / scale);
	slopes[2].dy = (int)((dy2 * 65536) / scale);
	slopes[2].k = newpoint.x * slopes[2].dy - newpoint.y * slopes[2].dx;

	/* use integer dx, dy in range -65536..65536 */
	slopes[3].dx = (int)((dx3 * 65536) / scale);
	slopes[3].dy = (int)((dy3 * 65536) / scale);
	slopes[3].k = newpoint.x * slopes[3].dy - newpoint.y * slopes[3].dx;
      }
      break;
    } /* end of switch */

  /* compute lists of left and right edges for the small polygon, using the
     just-computed slopes array */
  y = miPolyBuildPoly (vertices, slopes, edgecount, pLeft->x, pLeft->y,
		       left, right, &nleft, &nright, &height);
  /* fill the small polygon */
  miFillPolyHelper (paintedSet, pixel,
		    y, height, left, right, nleft, nright);
}

/* Paint either (1) a round cap on a line face or (2) a pie-wedge between
   two line faces.  Used for round capping and round joining respectively.
   One or two line faces are supplied.  They may be modified. */
static void
#ifdef _HAVE_PROTOS
miLineArc (miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, LineFace *leftFace, LineFace *rightFace, double xorg, double yorg, bool isInt)
#else
miLineArc (paintedSet, pixel, pGC, leftFace, rightFace, xorg, yorg, isInt)
     miPaintedSet *paintedSet;
     miPixel pixel;
     const miGC *pGC;
     LineFace *leftFace, *rightFace;
     double xorg, yorg;
     bool isInt;
#endif
{
  miPoint    *points;
  unsigned int  *widths;
  int           xorgi = 0, yorgi = 0;
  int 		n;
  PolyEdge	edge1, edge2;
  int		edgey1, edgey2;
  bool		edgeleft1, edgeleft2;

  if (isInt)
    /* in integer case, take (xorgi,yorgi) from face; otherwise (0,0) */
    {
      xorgi = leftFace ? leftFace->x : rightFace->x;
      yorgi = leftFace ? leftFace->y : rightFace->y;
    }

  edgey1 = INT_MAX;
  edgey2 = INT_MAX;
  edge1.x = 0;			/* not used, keep memory checkers happy */
  edge1.dy = -1;
  edge2.x = 0;			/* not used, keep memory checkers happy */
  edge2.dy = -1;
  edgeleft1 = false;
  edgeleft2 = false;
  if ((pGC->lineStyle != (int)MI_LINE_SOLID || pGC->lineWidth > 2)
      &&
      ((pGC->capStyle == (int)MI_CAP_ROUND && pGC->joinStyle != (int)MI_JOIN_ROUND) 
       ||
       (pGC->joinStyle == (int)MI_JOIN_ROUND && pGC->capStyle == (int)MI_CAP_BUTT)))
    /* construct clipping edges from the passed line faces (otherwise,
       ignore them; will just draw a disk) */
    {
      if (isInt)
	{
	  xorg = (double) xorgi;
	  yorg = (double) yorgi;
	}
      if (leftFace && rightFace)
	/* have two faces, so construct clipping edges for pie wedge */
	miRoundJoinClip (leftFace, rightFace, &edge1, &edge2,
			 &edgey1, &edgey2, &edgeleft1, &edgeleft2);

      else if (leftFace)
	/* will draw half-disk on left face, so construct clipping edge */
	edgey1 = miRoundCapClip (leftFace, isInt, &edge1, &edgeleft1);
      
      else if (rightFace)
	/* will draw half-disk on right face, so construct clipping edge */
	edgey2 = miRoundCapClip (rightFace, isInt, &edge2, &edgeleft2);

      /* due to clipping, switch to using floating-point coordinates */
      isInt = false;
    }

  points = (miPoint *)mi_xmalloc(sizeof(miPoint) * pGC->lineWidth);
  widths = (unsigned int *)mi_xmalloc(sizeof(unsigned int) * pGC->lineWidth);

  /* construct a Spans by calling integer or floating point routine */
  if (isInt)
    /* integer routine, no clipping: just draw a disk */
    n = miLineArcI (pGC, xorgi, yorgi, points, widths);
  else
    /* call floating point routine, supporting clipping by edge(s) */
    n = miLineArcD (pGC, xorg, yorg, points, widths,
		    &edge1, edgey1, edgeleft1,
		    &edge2, edgey2, edgeleft2);
  
  MI_PAINT_SPANS(paintedSet, pixel, n, points, widths)
}

/* Draw a filled disk, of diameter equal to the linewidth, as a Spans.
   This is used for round caps or round joins, if the clipping by one or
   two edges can be ignored.  Integer coordinates only are used.  Returns
   number of spans in the Spans. */
static int
#ifdef _HAVE_PROTOS
miLineArcI (const miGC *pGC, int xorg, int yorg, miPoint *points, unsigned int *widths)
#else
miLineArcI (pGC, xorg, yorg, points, widths)
     const miGC *pGC;
     int xorg, yorg;
     miPoint *points;
     unsigned int *widths;
#endif
{
  miPoint *tpts, *bpts;
  unsigned int *twids, *bwids;
  int x, y, e, ex;
  int slw;

  tpts = points;
  twids = widths;
  slw = (int)(pGC->lineWidth);
  if (slw == 1)
    /* `disk' is a single pixel */
    {
      tpts->x = xorg;
      tpts->y = yorg;
      *twids = 1;
      return 1;
    }

  /* otherwise, draw the disk scanline by scanline */
  bpts = tpts + slw;
  bwids = twids + slw;
  y = (slw >> 1) + 1;
  if (slw & 1)
    e = - ((y << 2) + 3);
  else
    e = - (y << 3);
  ex = -4;
  x = 0;
  while (y)
    {
      e += (y << 3) - 4;
      while (e >= 0)
	{
	  x++;
	  e += (ex = -((x << 3) + 4));
	}
      y--;
      slw = (x << 1) + 1;
      if ((e == ex) && (slw > 1))
	slw--;
      tpts->x = xorg - x;
      tpts->y = yorg - y;
      tpts++;
      *twids++ = slw;
      if ((y != 0) && ((slw > 1) || (e != ex)))
	{
	  bpts--;
	  bpts->x = xorg - x;
	  bpts->y = yorg + y;
	  *--bwids = slw;
	}
    }

  /* return linewidth (no. of spans in the Spans) */
  return (int)(pGC->lineWidth);
}

#define CLIPSTEPEDGE(edgey,edge,edgeleft) \
if (ybase == edgey) \
{ \
    if (edgeleft) \
      { \
	if (edge->x > xcl) \
	  xcl = edge->x; \
} \
  else \
    { \
      if (edge->x < xcr) \
	xcr = edge->x; \
} \
  edgey++; \
    edge->x += edge->stepx; \
      edge->e += edge->dx; \
	if (edge->e > 0) \
	  { \
	    edge->x += edge->signdx; \
	      edge->e -= edge->dy; \
} \
}

/* Draw as a Spans a filled disk of diameter equal to the linewidth, paying
   attention to one or two clipping edges.  This is used for round caps and
   round joins, respectively (it respectively yields a half-disk or a pie
   wedge).  Floating point coordinates are used.  Returns number of spans
   in the Spans.  The clipping edges may be modified. */
static int
#ifdef _HAVE_PROTOS
miLineArcD (const miGC *pGC, double xorg, double yorg, miPoint *points, unsigned int *widths, PolyEdge *edge1, int edgey1, bool edgeleft1, PolyEdge *edge2, int edgey2, bool edgeleft2)
#else
miLineArcD (pGC, xorg, yorg, points, widths, edge1, edgey1, edgeleft1, edge2, edgey2, edgeleft2)
     const miGC *pGC;
     double xorg, yorg;
     miPoint *points;
     unsigned int *widths;
     PolyEdge *edge1;
     int edgey1;
     bool edgeleft1;
     PolyEdge *edge2;
     int edgey2;
     bool edgeleft2;
#endif
{
  miPoint *pts;
  unsigned int *wids;
  double radius, x0, y0, el, er, yk, xlk, xrk, k;
  int xbase, ybase, y, boty, xl, xr, xcl, xcr;
  int ymin, ymax;
  bool edge1IsMin, edge2IsMin;
  int ymin1, ymin2;

  pts = points;
  wids = widths;
  xbase = (int)(floor(xorg));
  x0 = xorg - xbase;
  ybase = ICEIL (yorg);
  y0 = yorg - ybase;
  xlk = x0 + x0 + 1.0;
  xrk = x0 + x0 - 1.0;
  yk = y0 + y0 - 1.0;
  radius = 0.5 * ((double)pGC->lineWidth);
  y = (int)(floor(radius - y0 + 1.0));
  ybase -= y;
  ymin = ybase;
  ymax = INT_MAX;
  edge1IsMin = false;
  ymin1 = edgey1;
  if (edge1->dy >= 0)
    {
      if (!edge1->dy)
    	{
	  if (edgeleft1)
	    edge1IsMin = true;
	  else
	    ymax = edgey1;
	  edgey1 = INT_MAX;
    	}
      else
    	{
	  if ((edge1->signdx < 0) == edgeleft1)
	    edge1IsMin = true;
    	}
    }
  edge2IsMin = false;
  ymin2 = edgey2;
  if (edge2->dy >= 0)
    {
      if (!edge2->dy)
    	{
	  if (edgeleft2)
	    edge2IsMin = true;
	  else
	    ymax = edgey2;
	  edgey2 = INT_MAX;
    	}
      else
    	{
	  if ((edge2->signdx < 0) == edgeleft2)
	    edge2IsMin = true;
    	}
    }
  if (edge1IsMin)
    {
      ymin = ymin1;
      if (edge2IsMin && ymin1 > ymin2)
	ymin = ymin2;
    } 
  else if (edge2IsMin)
    ymin = ymin2;
  el = radius * radius - ((y + y0) * (y + y0)) - (x0 * x0);
  er = el + xrk;
  xl = 1;
  xr = 0;
  if (x0 < 0.5)
    {
      xl = 0;
      el -= xlk;
    }
  boty = (y0 < -0.5) ? 1 : 0;
  if (ybase + y - boty > ymax)
    boty = ymax - ybase - y;
  while (y > boty)
    {
      k = (y << 1) + yk;
      er += k;
      while (er > 0.0)
	{
	  xr++;
	  er += xrk - (xr << 1);
	}
      el += k;
      while (el >= 0.0)
	{
	  xl--;
	  el += (xl << 1) - xlk;
	}
      y--;
      ybase++;
      if (ybase < ymin)
	continue;
      xcl = xl + xbase;
      xcr = xr + xbase;
      CLIPSTEPEDGE(edgey1, edge1, edgeleft1);
      CLIPSTEPEDGE(edgey2, edge2, edgeleft2);
      if (xcr >= xcl)
	{
	  pts->x = xcl;
	  pts->y = ybase;
	  pts++;
	  *wids++ = (unsigned int)(xcr - xcl + 1);
	}
    }
  er = xrk - (xr << 1) - er;
  el = (xl << 1) - xlk - el;
  boty = (int)(floor(-y0 - radius + 1.0));
  if (ybase + y - boty > ymax)
    boty = ymax - ybase - y;
  while (y > boty)
    {
      k = (y << 1) + yk;
      er -= k;
      while ((er >= 0.0) && (xr >= 0))
	{
	  xr--;
	  er += xrk - (xr << 1);
	}
      el -= k;
      while ((el > 0.0) && (xl <= 0))
	{
	  xl++;
	  el += (xl << 1) - xlk;
	}
      y--;
      ybase++;
      if (ybase < ymin)
	continue;
      xcl = xl + xbase;
      xcr = xr + xbase;
      CLIPSTEPEDGE(edgey1, edge1, edgeleft1);
      CLIPSTEPEDGE(edgey2, edge2, edgeleft2);
      if (xcr >= xcl)
	{
	  pts->x = xcl;
	  pts->y = ybase;
	  pts++;
	  *wids++ = (unsigned int)(xcr - xcl + 1);
	}
    }

  /* return number of spans in the Spans */
  return (pts - points);
}

/* From two line faces, construct clipping edges that will be used by
   miLineArcD when drawing a pie wedge.  The line faces may be modified. */
static void
#ifdef _HAVE_PROTOS
miRoundJoinClip (LineFace *pLeft, LineFace *pRight, PolyEdge *edge1, PolyEdge *edge2, int *y1, int *y2, bool *left1, bool *left2)
#else
miRoundJoinClip (pLeft, pRight, edge1, edge2, y1, y2, left1, left2)
     LineFace *pLeft, *pRight;
     PolyEdge *edge1, *edge2;
     int *y1, *y2;
     bool *left1, *left2;
#endif
{
  int	denom;

  denom = - pLeft->dx * pRight->dy + pRight->dx * pLeft->dy;
  if (denom >= 0)
    {
      pLeft->xa = -pLeft->xa;
      pLeft->ya = -pLeft->ya;
    }
  else
    {
      pRight->xa = -pRight->xa;
      pRight->ya = -pRight->ya;
    }
  *y1 = miRoundJoinFace (pLeft, edge1, left1);
  *y2 = miRoundJoinFace (pRight, edge2, left2);
}

/* helper function called by the preceding */
static int
#ifdef _HAVE_PROTOS
miRoundJoinFace (const LineFace *face, PolyEdge *edge, bool *leftEdge)
#else
miRoundJoinFace (face, edge, leftEdge)
     const LineFace *face;
     PolyEdge *edge;
     bool *leftEdge;
#endif
{
  int	    y;
  int	    dx, dy;
  double    xa, ya;
  bool	    left;

  dx = -face->dy;
  dy = face->dx;
  xa = face->xa;
  ya = face->ya;
  left = true;
  if (ya > 0)
    {
      ya = 0.0;
      xa = 0.0;
    }
  if (dy < 0 || (dy == 0 && dx > 0))
    {
      dx = -dx;
      dy = -dy;
      left = (left ? false : true);
    }
  if (dx == 0 && dy == 0)
    dy = 1;
  if (dy == 0)
    {
      y = ICEIL (face->ya) + face->y;
      edge->x = INT_MIN;
      edge->stepx = 0;
      edge->signdx = 0;
      edge->e = -1;
      edge->dy = 0;
      edge->dx = 0;
      edge->height = 0;
    }
  else
    {
      y = miPolyBuildEdge (xa, ya, 
			   0.0, dx, dy, 
			   face->x, face->y, (left ? false : true), edge);
      edge->height = UINT_MAX;	/* number of scanlines to process */
    }
  *leftEdge = (left ? false : true);

  return y;
}

/* From a line face, construct a clipping edge that will be used by
   miLineArcD when drawing a half-disk.  */
static int
#ifdef _HAVE_PROTOS
miRoundCapClip (const LineFace *face, bool isInt, PolyEdge *edge, bool *leftEdge)
#else
miRoundCapClip (face, isInt, edge, leftEdge)
     const LineFace *face;
     bool isInt;
     PolyEdge *edge;
     bool *leftEdge;
#endif
{
  int	    y;
  int 	    dx, dy;
  double    xa, ya, k;
  bool	    left;

  dx = -face->dy;
  dy = face->dx;
  xa = face->xa;
  ya = face->ya;
  k = 0.0;
  if (!isInt)
    k = face->k;
  left = true;
  if (dy < 0 || (dy == 0 && dx > 0))
    {
      dx = -dx;
      dy = -dy;
      xa = -xa;
      ya = -ya;
      left = (left ? false : true);
    }
  if (dx == 0 && dy == 0)
    dy = 1;
  if (dy == 0)
    {
      y = ICEIL (face->ya) + face->y;
      edge->x = INT_MIN;
      edge->stepx = 0;
      edge->signdx = 0;
      edge->e = -1;
      edge->dy = 0;
      edge->dx = 0;
      edge->height = 0;
    }
  else
    {
      y = miPolyBuildEdge (xa, ya,
			   k, dx, dy,
			   face->x, face->y, (left ? false : true), edge);
      edge->height = UINT_MAX;	/* number of scanlines to process */
    }
  *leftEdge = (left ? false : true);

  return y;
}

/* Paint a projecting rectangular cap on a line face.  Called only by
   miWideDash (with isInt = true); not by miWideLine. */
static void
#ifdef _HAVE_PROTOS
miLineProjectingCap (miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, const LineFace *face, bool isLeft, bool isInt)
#else
miLineProjectingCap (paintedSet, pixel, pGC, face, isLeft, isInt)
     miPaintedSet *paintedSet;
     miPixel pixel;
     const miGC *pGC;
     const LineFace *face;
     bool isLeft;
     bool isInt;
#endif
{
  int		xorgi = 0, yorgi = 0;
  int	       	lw;
  PolyEdge	lefts[2], rights[2];
  int		lefty, righty, topy, bottomy;
  PolyEdge      *left, *right;
  PolyEdge      *top, *bottom;
  double	xa,ya;
  double	k;
  double	xap, yap;
  int		dx, dy;
  double	projectXoff, projectYoff;
  double	maxy;
  int		finaly;
    
  if (isInt)
    /* in integer case, take (xorgi,yorgi) from face; otherwise (0,0) */
    {
      xorgi = face->x;
      yorgi = face->y;
    }
  lw = (int)(pGC->lineWidth);
  dx = face->dx;
  dy = face->dy;
  k = face->k;
  if (dy == 0)
    /* special case: line face is horizontal */
    {
      lefts[0].height = (unsigned int)lw;
      lefts[0].x = xorgi;
      if (isLeft)
	lefts[0].x -= (lw >> 1);
      lefts[0].stepx = 0;
      lefts[0].signdx = 1;
      lefts[0].e = -lw;
      lefts[0].dx = 0;
      lefts[0].dy = lw;

      rights[0].height = (unsigned int)lw;
      rights[0].x = xorgi;
      if (!isLeft)
	rights[0].x += ((lw + 1) >> 1);
      rights[0].stepx = 0;
      rights[0].signdx = 1;
      rights[0].e = -lw;
      rights[0].dx = 0;
      rights[0].dy = lw;

      /* fill the rectangle (1 left edge, 1 right edge) */
      miFillPolyHelper (paintedSet, pixel,
			yorgi - (lw >> 1), (unsigned int)lw, 
			lefts, rights, 1, 1);
    }
  else if (dx == 0)
    /* special case: line face is vertical */
    {
      topy = yorgi;
      bottomy = yorgi + dy;
      if (isLeft)
	topy -= (lw >> 1);
      else
	bottomy += (lw >> 1);
      lefts[0].height = (unsigned int)(bottomy - topy);
      lefts[0].x = xorgi - (lw >> 1);
      lefts[0].stepx = 0;
      lefts[0].signdx = 1;
      lefts[0].e = -dy;
      lefts[0].dx = dx;
      lefts[0].dy = dy;

      rights[0].height = (unsigned int)(bottomy - topy);
      rights[0].x = lefts[0].x + (lw - 1);
      rights[0].stepx = 0;
      rights[0].signdx = 1;
      rights[0].e = -dy;
      rights[0].dx = dx;
      rights[0].dy = dy;

      /* fill the rectangle (1 left edge, 1 right edge) */
      miFillPolyHelper (paintedSet, pixel, topy, 
			(unsigned int)(bottomy - topy), lefts, rights, 1, 1);
    }
  else
    /* general case: line face is neither horizontal nor vertical */
    {
      xa = face->xa;
      ya = face->ya;
      projectXoff = -ya;
      projectYoff = xa;
      if (dx < 0)
	{
	  right = &rights[1];
	  left = &lefts[0];
	  top = &rights[0];
	  bottom = &lefts[1];
	}
      else
	{
	  right = &rights[0];
	  left = &lefts[1];
	  top = &lefts[0];
	  bottom = &rights[1];
	}
      if (isLeft)
	/* cap goes left; build four edges */
	{
	  righty = miPolyBuildEdge (xa, ya,
				    k, dx, dy, 
				    xorgi, yorgi, false, right);
	    
	  xa = -xa;
	  ya = -ya;
	  k = -k;
	  lefty = miPolyBuildEdge (xa - projectXoff, ya - projectYoff,
				   k, dx, dy, 
				   xorgi, yorgi, true, left);
	  if (dx > 0)
	    {
	      ya = -ya;
	      xa = -xa;
	    }
	  xap = xa - projectXoff;
	  yap = ya - projectYoff;
	  topy = miPolyBuildEdge (xap, yap, 
				  xap * dx + yap * dy, -dy, dx,
				  xorgi, yorgi, (dx > 0 ? true : false), top);
	  bottomy = miPolyBuildEdge (xa, ya,
				     0.0, -dy, dx, 
				     xorgi, yorgi, (dx < 0 ? true : false), bottom);
	  maxy = -ya;
	}
      else
	/* cap goes right; build four edges */
	{
	  righty = miPolyBuildEdge (xa - projectXoff, ya - projectYoff,
				    k, dx, dy, 
				    xorgi, yorgi, false, right);
	    
	  xa = -xa;
	  ya = -ya;
	  k = -k;
	  lefty = miPolyBuildEdge (xa, ya,
				   k, dx, dy, 
				   xorgi, yorgi, true, left);
	  if (dx > 0)
	    {
	      ya = -ya;
	      xa = -xa;
	    }
	  xap = xa - projectXoff;
	  yap = ya - projectYoff;
	  topy = miPolyBuildEdge (xa, ya, 
				  0.0, -dy, dx,
				  xorgi, xorgi, (dx > 0 ? true : false), top);
	  bottomy = miPolyBuildEdge (xap, yap, 
				     xap * dx + yap * dy, -dy, dx,
				     xorgi, xorgi, (dx < 0 ? true : false), bottom);
	  maxy = -ya + projectYoff;
	}

      finaly = ICEIL(maxy) + yorgi;
      if (dx < 0)
	{
	  left->height = (unsigned int)(bottomy - lefty);
	  right->height = (unsigned int)(finaly - righty);
	  top->height = (unsigned int)(righty - topy);
	}
      else
	{
	  right->height = (unsigned int)(bottomy - righty);
	  left->height = (unsigned int)(finaly - lefty);
	  top->height = (unsigned int)(lefty - topy);
	}
      bottom->height = (unsigned int)(finaly - bottomy);

      /* fill the rectangle (2 left edges, 2 right edges) */
      miFillPolyHelper (paintedSet, pixel, topy,
			(unsigned int)(bottom->height + bottomy - topy),
			lefts, rights, 2, 2);
    }
}


/* Draw a wide, dashed polyline, by dashing each line segment and joining
   appropriately.  miWideDashSegment() is called to dash each line
   segment. */
void
#ifdef _HAVE_PROTOS
miWideDash (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
#else
miWideDash (paintedSet, pGC, mode, npt, pPts)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     miCoordMode mode;
     int npt;
     const miPoint *pPts;
#endif
{
  int	    x1, y1, x2, y2;
  int	    dashNum;		/* absolute number of dash, starts with 0 */
  int       dashIndex;		/* index into array (i.e. dashNum % length) */
  int       dashOffset;		/* offset into selected dash */
  int       startPaintType, endPaintType = 0, prevEndPaintType = 0;
  int       firstPaintType = 0;	/* used only for closed polylines; will be 1 */
  int       numPixels;
  bool	    selfJoin;		/* polyline is closed? */
  bool	    first;		/* first line segment of polyline */
  bool	    somethingDrawn = false;
  bool	    projectLeft, projectRight;
  LineFace  leftFace, rightFace, prevRightFace;
  LineFace  firstFace;
  miPixel   pixel;

  /* ensure we have >=1 points */
  if (npt <= 0)
    return;

  /* width 0 lines are handled specially; invoke Bresenham routine in
     mi_zerolin.c */
  if (pGC->lineWidth == 0)
    {
      miZeroDash (paintedSet, pGC, mode, npt, pPts);
      return;
    }

  x2 = pPts->x;
  y2 = pPts->y;
  first = true;			/* first line segment of polyline */

  /* determine whether polyline is closed */
  selfJoin = false;
  if (mode == MI_COORD_MODE_PREVIOUS)
    {
      int nptTmp;
      const miPoint *pPtsTmp;

      x1 = x2;
      y1 = y2;
      nptTmp = npt;
      pPtsTmp = pPts + 1;
      while (--nptTmp)
	{
	  x1 += pPtsTmp->x;
	  y1 += pPtsTmp->y;
	  ++pPtsTmp;
	}
      if (x2 == x1 && y2 == y1)
	selfJoin = true;
    }
  else if (x2 == pPts[npt-1].x && y2 == pPts[npt-1].y)
    selfJoin = true;

  /* dash segments (except for the last) will not project right; and
     (except for the first) will not project left */
  projectLeft = 
    (pGC->capStyle == (int)MI_CAP_PROJECTING && !selfJoin) ? true : false;
  projectRight = false;

  /* perform initial offsetting into the dash sequence */
  dashNum = 0;			/* absolute number of dash */
  dashIndex = 0;		/* index into dash array */
  dashOffset = 0;		/* index into selected dash  */
  miStepDash (pGC->dashOffset, &dashNum, &dashIndex,
	      pGC->dash, pGC->numInDashList, &dashOffset);

  /* How many paint types?  (Will cycle through 0..numPixels-1, beginning
     with 1, with `off' dashes defined as those with paint type #0.) */
  numPixels = pGC->numPixels;

  /* iterate through points, drawing a dashed segment for each line segment
     of nonzero length */
  while (--npt)
    {
      x1 = x2;
      y1 = y2;
      ++pPts;
      x2 = pPts->x;
      y2 = pPts->y;
      if (mode == MI_COORD_MODE_PREVIOUS)
	{
	  x2 += x1;
	  y2 += y1;
	}

      if (x1 != x2 || y1 != y2)
	/* have a line segment of nonzero length */
	{
	  int prevDashNum, lastPaintedDashNum;

	  if (npt == 1 && pGC->capStyle == (int)MI_CAP_PROJECTING 
	      && (!selfJoin || (firstPaintType == 0)))
	    /* final point; and need a projecting cap here */
	    projectRight = true;
	  prevDashNum = dashNum;
	  /* draw dashed segment, updating dashNum, dashIndex and
             dashOffset, returning faces */
	  miWideDashSegment (paintedSet, pGC, 
			     &dashNum, &dashIndex, &dashOffset,
			     x1, y1, x2, y2,
			     projectLeft, projectRight, &leftFace, &rightFace);

	  /* determine paint types used at start and end of just-drawn
	     segment */
	  startPaintType = ((dashNum & 1) ? 
			    0 : 1 + ((dashNum / 2) % (numPixels - 1)));
	  lastPaintedDashNum = (dashOffset != 0 ? dashNum : dashNum - 1);
	  endPaintType = ((lastPaintedDashNum & 1) ? 
			    0 : 1 + ((dashNum / 2) % (numPixels - 1)));

	  /* add round cap or line join at left end of just-drawn segment;
	     if OnOffDash, do so only if segment began with an `on' dash */
	  if (pGC->lineStyle == (int)MI_LINE_DOUBLE_DASH || (startPaintType != 0))
	    {
	      pixel = pGC->pixels[startPaintType];
	      if (first || (pGC->lineStyle == (int)MI_LINE_ON_OFF_DASH 
			    && prevEndPaintType == 0))
		/* draw cap at left end, unless this is first segment of a
                   closed polyline */
	    	{
		  if (first && selfJoin)
		    {
		      firstFace = leftFace;
		      firstPaintType = startPaintType;
		    }
		  else if (pGC->capStyle == (int)MI_CAP_ROUND
			   || pGC->capStyle == (int)MI_CAP_TRIANGULAR)
		    /* invoke miLineArc to draw round cap, isInt = true */
		    miLineArc (paintedSet, pixel, pGC,
			       &leftFace, (LineFace *)NULL,
			       (double)0.0, (double)0.0, true);
	    	}
	      else
		/* draw join at left end */
		  miLineJoin (paintedSet, pixel, pGC,
			      &leftFace, &prevRightFace);
	    }

	  somethingDrawn = true;
	  first = false;
	  prevRightFace = rightFace;
	  prevEndPaintType = endPaintType;
	  projectLeft = false;
	}

      if (npt == 1 && somethingDrawn)
	/* last point of a nonempty polyline, so add line join or round cap
	   if appropriate, i.e. if we're doing OnOffDash and ended on an
	   `on' dash, or if we're doing DoubleDash */
	{
	  if (pGC->lineStyle == (int)MI_LINE_DOUBLE_DASH || (endPaintType != 0))
	    {
	      pixel = pGC->pixels[endPaintType];
	      if (selfJoin && (pGC->lineStyle == (int)MI_LINE_DOUBLE_DASH 
			       || (firstPaintType != 0)))
		/* closed, so draw a join */
		miLineJoin (paintedSet, pixel, pGC,
			    &firstFace, &rightFace);
	      else 
		{
		  if (pGC->capStyle == (int)MI_CAP_ROUND
		      || pGC->capStyle == (int)MI_CAP_TRIANGULAR)
		    /* invoke miLineArc, isInt = true, to draw a round cap */
		    miLineArc (paintedSet, pixel, pGC,
			       (LineFace *)NULL, &rightFace,
			       (double)0.0, (double)0.0, true);
		}
	    }
	  else
	    /* we're doing OnOffDash, and final segment of polyline ended
	       with an (undrawn) `off' dash */
	    {
	      if (selfJoin && (firstPaintType != 0))
		/* closed; if projecting or round caps are being used, draw
		   one on the first face */
		{
		  pixel = pGC->pixels[firstPaintType];
		  if (pGC->capStyle == (int)MI_CAP_PROJECTING)
		    miLineProjectingCap (paintedSet, pixel, pGC,
					 &firstFace, true, true);
		  else if (pGC->capStyle == (int)MI_CAP_ROUND
			   || pGC->capStyle == (int)MI_CAP_TRIANGULAR)
		    /* invoke miLineArc, isInt = true, to draw a round cap */
		    miLineArc (paintedSet, pixel, pGC,
			       &firstFace, (LineFace *)NULL,
			       (double)0.0, (double)0.0, true);
		}
	    }
	}
    }

  /* handle `all points coincident' crock, nothing yet drawn */
  if (!somethingDrawn 
      && (pGC->lineStyle == (int)MI_LINE_DOUBLE_DASH || !(dashNum & 1)))
    {
      unsigned int w1;

      pixel = (dashNum & 1) ? pGC->pixels[0] : pGC->pixels[1];
      switch ((int)pGC->capStyle) 
	{
	case (int)MI_CAP_ROUND:
	case (int)MI_CAP_TRIANGULAR:
	  /* invoke miLineArc, isInt = false, to draw a round disk */
	  miLineArc (paintedSet, pixel, pGC,
		     (LineFace *)NULL, (LineFace *)NULL,
		     (double)x2, (double)y2,
		     false);
	  break;
	case (int)MI_CAP_PROJECTING:
	  /* draw a square box with edge size equal to line width */
	  w1 = pGC->lineWidth;
	  miFillRectPolyHelper (paintedSet, pixel,
				(int)(x2 - (w1 >> 1)), (int)(y2 - (w1 >> 1)),
				w1, w1);
	  break;
	case (int)MI_CAP_BUTT:
	default:
	  break;
	}
    }
}


#define V_TOP	    0
#define V_RIGHT	    1
#define V_BOTTOM    2
#define V_LEFT	    3

/* Helper function, called by miWideDash().  Draw a single dashed line
   segment, i.e. a sequence of dashes including a possible final incomplete
   dash, and step DashNum, DashIndex and DashOffset appropriately.  Also
   pass back left and right faces for the line segment, for possible use in
   adding caps or joins.  If the LineOnOffDash line style is used, each
   dash will be given a round cap if lines are drawn in the rounded cap
   style, and a projecting cap if lines are drawn in the projecting cap
   style. */
static void
#ifdef _HAVE_PROTOS
miWideDashSegment (miPaintedSet *paintedSet, const miGC *pGC, int *pDashNum, int *pDashIndex, int *pDashOffset, int x1, int y1, int x2, int y2, bool projectLeft, bool projectRight, LineFace *leftFace, LineFace *rightFace)
#else
miWideDashSegment (paintedSet, pGC, pDashNum, pDashIndex, pDashOffset, x1, y1, x2, y2, projectLeft, projectRight, leftFace, rightFace)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int *pDashNum;		/* absolute number of dash */
     int *pDashIndex;		/* index into array (i.e. dashNum % length) */
     int *pDashOffset;		/* offset into selected dash */
     int x1, y1, x2, y2;
     bool projectLeft, projectRight;
     LineFace *leftFace, *rightFace;
#endif
{
  int		    dashNum, dashIndex, dashRemain;
  unsigned int      *pDash;
  double	    L, l;
  double	    k;
  PolyVertex	    vertices[4];
  PolyVertex	    saveRight, saveBottom;
  PolySlope	    slopes[4];
  PolyEdge	    left[2], right[2];
  LineFace	    lcapFace, rcapFace;
  int		    nleft, nright;
  unsigned int	    h;
  int		    y;
  int		    dy, dx;
  double	    LRemain;
  double	    r;
  double	    rdx, rdy;
  double	    dashDx, dashDy;
  double	    saveK = 0.0;
  bool	    	    first = true;
  double	    lcenterx, lcentery, rcenterx = 0.0, rcentery = 0.0;
  miPixel	    pixel;
  int    	    numPixels, paintType;
    
  dx = x2 - x1;
  dy = y2 - y1;
  dashNum = *pDashNum;
  dashIndex = *pDashIndex;
  pDash = pGC->dash;
  /* determine portion of current dash remaining (i.e. the portion after
     the current offset */
  dashRemain = (int)(pDash[dashIndex]) - *pDashOffset;	

  /* compute color of current dash */
  numPixels = pGC->numPixels;
  paintType = (dashNum & 1) ? 0 : 1 + ((dashNum / 2) % (numPixels - 1));
  pixel = pGC->pixels[paintType];

  /* compute e.g. L, the distance to go (for dashing) */
  l = 0.5 * ((double) pGC->lineWidth);
  if (dx == 0)			/* vertical segment */
    {
      L = dy;
      rdx = 0;
      rdy = l;
      if (dy < 0)
	{
	  L = -dy;
	  rdy = -l;
	}
    }
  else if (dy == 0)		/* horizontal segment */
    {
      L = dx;
      rdx = l;
      rdy = 0;
      if (dx < 0)
	{
	  L = -dx;
	  rdx = -l;
	}
    }
  else				/* neither horizontal nor vertical */
    {
      L = hypot ((double) dx, (double) dy);
      r = l / L;		/* this is ell / L, not 1 / L */
      rdx = r * dx;
      rdy = r * dy;
    }
  k = l * L;			/* this is ell * L, not 1 * L */

  /* All position comments are relative to a line with dx and dy > 0,
   * but the code does not depend on this. */
  /* top */
  slopes[V_TOP].dx = dx;
  slopes[V_TOP].dy = dy;
  slopes[V_TOP].k = k;
  /* right */
  slopes[V_RIGHT].dx = -dy;
  slopes[V_RIGHT].dy = dx;
  slopes[V_RIGHT].k = 0;
  /* bottom */
  slopes[V_BOTTOM].dx = -dx;
  slopes[V_BOTTOM].dy = -dy;
  slopes[V_BOTTOM].k = k;
  /* left */
  slopes[V_LEFT].dx = dy;
  slopes[V_LEFT].dy = -dx;
  slopes[V_LEFT].k = 0;

  /* preload the start coordinates */
  vertices[V_RIGHT].x = vertices[V_TOP].x = rdy;
  vertices[V_RIGHT].y = vertices[V_TOP].y = -rdx;

  vertices[V_BOTTOM].x = vertices[V_LEFT].x = -rdy;
  vertices[V_BOTTOM].y = vertices[V_LEFT].y = rdx;

  if (projectLeft)
    /* offset the vertices appropriately */
    {
      vertices[V_TOP].x -= rdx;
      vertices[V_TOP].y -= rdy;

      vertices[V_LEFT].x -= rdx;
      vertices[V_LEFT].y -= rdy;

      slopes[V_LEFT].k = rdx * dx + rdy * dy;
    }

  /* starting point for first dash (floating point) */
  lcenterx = x1;
  lcentery = y1;

  if (pGC->capStyle == (int)MI_CAP_ROUND
      || pGC->capStyle == (int)MI_CAP_TRIANGULAR)
    /* keep track of starting face (need only in OnOff case) */
    {
      lcapFace.dx = dx;
      lcapFace.dy = dy;
      lcapFace.x = x1;
      lcapFace.y = y1;

      rcapFace.dx = -dx;
      rcapFace.dy = -dy;
      rcapFace.x = x1;
      rcapFace.y = y1;
    }

  /* draw dashes until end of line segment is reached, and no additional
     (complete) dash can be drawn */
  LRemain = L;
  while (LRemain > dashRemain)
    {
      dashDx = (dashRemain * dx) / L;
      dashDy = (dashRemain * dy) / L;

      /* ending point for dash */
      rcenterx = lcenterx + dashDx;
      rcentery = lcentery + dashDy;

      vertices[V_RIGHT].x += dashDx;
      vertices[V_RIGHT].y += dashDy;

      vertices[V_BOTTOM].x += dashDx;
      vertices[V_BOTTOM].y += dashDy;

      slopes[V_RIGHT].k = vertices[V_RIGHT].x * dx + vertices[V_RIGHT].y * dy;

      /* draw dash (if OnOffDash, don't draw `off' dashes) */
      if (pGC->lineStyle == (int)MI_LINE_DOUBLE_DASH || !(paintType == 0))
	{
	  if (pGC->lineStyle == (int)MI_LINE_ON_OFF_DASH &&
	      pGC->capStyle == (int)MI_CAP_PROJECTING)
	    /* will draw projecting caps, so save vertices for later use */
	    {
	      saveRight = vertices[V_RIGHT];
	      saveBottom = vertices[V_BOTTOM];
	      saveK = slopes[V_RIGHT].k;
		
	      if (!first)
		{
		  vertices[V_TOP].x -= rdx;
		  vertices[V_TOP].y -= rdy;
    
		  vertices[V_LEFT].x -= rdx;
		  vertices[V_LEFT].y -= rdy;

		  slopes[V_LEFT].k = vertices[V_LEFT].x *
		    slopes[V_LEFT].dy -
		      vertices[V_LEFT].y *
			slopes[V_LEFT].dx;
		}
		
	      vertices[V_RIGHT].x += rdx;
	      vertices[V_RIGHT].y += rdy;

	      vertices[V_BOTTOM].x += rdx;
	      vertices[V_BOTTOM].y += rdy;

	      slopes[V_RIGHT].k = vertices[V_RIGHT].x *
		slopes[V_RIGHT].dy -
		  vertices[V_RIGHT].y *
		    slopes[V_RIGHT].dx;
	    }

	  /* build lists of left and right edges for the dash, using the
	     just-computed array of slopes */
	  y = miPolyBuildPoly (vertices, slopes, 4, x1, y1,
			       left, right, &nleft, &nright, &h);

	  /* fill the dash, with either fg or bg color (alternates) */
	  miFillPolyHelper (paintedSet, pixel, 
			    y, h, left, right, nleft, nright);

	  if (pGC->lineStyle == (int)MI_LINE_ON_OFF_DASH)
	    /* if doing OnOffDash, add caps if any */
	    {
	      switch ((int)pGC->capStyle)
		{
		case (int)MI_CAP_BUTT:
		default:
		  break;
		case (int)MI_CAP_PROJECTING:
		  /* use saved vertices */
		  vertices[V_BOTTOM] = saveBottom;
		  vertices[V_RIGHT] = saveRight;
		  slopes[V_RIGHT].k = saveK;
		  break;
		case (int)MI_CAP_ROUND:
		case (int)MI_CAP_TRIANGULAR:
		  if (!first)
		    {
		      if (dx < 0)
		    	{
			  lcapFace.xa = -vertices[V_LEFT].x;
			  lcapFace.ya = -vertices[V_LEFT].y;
			  lcapFace.k = slopes[V_LEFT].k;
		    	}
		      else
		    	{
			  lcapFace.xa = vertices[V_TOP].x;
			  lcapFace.ya = vertices[V_TOP].y;
			  lcapFace.k = -slopes[V_LEFT].k;
		    	}
		      /* invoke miLineArc, isInt = false, to draw half-disk
			 on left end of dash (only if dash is not first) */
		      miLineArc (paintedSet, pixel, pGC,
				 &lcapFace, (LineFace *) NULL,
				 lcenterx, lcentery, false);
		    }
		  if (dx < 0)
		    {
		      rcapFace.xa = vertices[V_BOTTOM].x;
		      rcapFace.ya = vertices[V_BOTTOM].y;
		      rcapFace.k = slopes[V_RIGHT].k;
		    }
		  else
		    {
		      rcapFace.xa = -vertices[V_RIGHT].x;
		      rcapFace.ya = -vertices[V_RIGHT].y;
		      rcapFace.k = -slopes[V_RIGHT].k;
		    }
		  /* invoke miLineArc, isInt = false, to draw half-disk on
		     right end of dash */
		  miLineArc (paintedSet, pixel, pGC,
			     (LineFace *)NULL, &rcapFace,
			     rcenterx, rcentery, false);
		  break;
	    	}
	    }
	}

      /* we just drew a dash, or (in the OnOff case) we either drew a dash
	 or we didn't */

      LRemain -= dashRemain;	/* decrement float by int (distance over
				   which we just drew, i.e. the remainder
				   of current dash) */

      /* bump absolute dash number, and index of dash in array (cyclically) */
      ++dashNum;
      ++dashIndex;
      if (dashIndex == pGC->numInDashList)
	dashIndex = 0;
      dashRemain = (int)(pDash[dashIndex]); /* whole new dash now `remains' */

      /* compute color of next dash */
      paintType = (dashNum & 1) ? 0 : 1 + ((dashNum / 2) % (numPixels - 1));
      pixel = pGC->pixels[paintType];

      /* next dash will start where previous one ended */
      lcenterx = rcenterx;
      lcentery = rcentery;

      vertices[V_TOP] = vertices[V_RIGHT];
      vertices[V_LEFT] = vertices[V_BOTTOM];
      slopes[V_LEFT].k = -slopes[V_RIGHT].k;
      first = false;		/* no longer first dash of line segment */
    }

  /* final portion of segment is dashed specially, with an incomplete dash */
  if (pGC->lineStyle == (int)MI_LINE_DOUBLE_DASH || !(paintType == 0))
    {
      vertices[V_TOP].x -= dx;
      vertices[V_TOP].y -= dy;

      vertices[V_LEFT].x -= dx;
      vertices[V_LEFT].y -= dy;

      vertices[V_RIGHT].x = rdy;
      vertices[V_RIGHT].y = -rdx;

      vertices[V_BOTTOM].x = -rdy;
      vertices[V_BOTTOM].y = rdx;
	
      if (projectRight)
	/* offset appropriately */
	{
	  vertices[V_RIGHT].x += rdx;
	  vertices[V_RIGHT].y += rdy;
    
	  vertices[V_BOTTOM].x += rdx;
	  vertices[V_BOTTOM].y += rdy;
	  slopes[V_RIGHT].k = vertices[V_RIGHT].x *
	    slopes[V_RIGHT].dy -
	      vertices[V_RIGHT].y *
		slopes[V_RIGHT].dx;
	}
      else
	slopes[V_RIGHT].k = 0;

      /* if OnOffDash line style and cap mode is projecting, offset the
	 face, so as to draw a projecting cap */
      if (!first && pGC->lineStyle == (int)MI_LINE_ON_OFF_DASH 
	  && pGC->capStyle == (int)MI_CAP_PROJECTING)
	{
	  vertices[V_TOP].x -= rdx;
	  vertices[V_TOP].y -= rdy;
	  
	  vertices[V_LEFT].x -= rdx;
	  vertices[V_LEFT].y -= rdy;
	  slopes[V_LEFT].k = vertices[V_LEFT].x *
	    slopes[V_LEFT].dy -
	      vertices[V_LEFT].y *
		slopes[V_LEFT].dx;
	}
      else
	slopes[V_LEFT].k += dx * dx + dy * dy;
      
      /* build lists of left and right edges for the final incomplete dash,
	 using the just-computed vertices and slopes */
      y = miPolyBuildPoly (vertices, slopes, 4, x2, y2,
			   left, right, &nleft, &nright, &h);

      /* fill the final dash */
      miFillPolyHelper (paintedSet, pixel,
			y, h, left, right, nleft, nright);

      /* if OnOffDash line style and cap mode is round, draw a round cap */
      if (!first && pGC->lineStyle == (int)MI_LINE_ON_OFF_DASH
	  && (pGC->capStyle == (int)MI_CAP_ROUND
	      || pGC->capStyle == (int)MI_CAP_TRIANGULAR))
	{
	  lcapFace.x = x2;
	  lcapFace.y = y2;
	  if (dx < 0)
	    {
	      lcapFace.xa = -vertices[V_LEFT].x;
	      lcapFace.ya = -vertices[V_LEFT].y;
	      lcapFace.k = slopes[V_LEFT].k;
	    }
	  else
	    {
	      lcapFace.xa = vertices[V_TOP].x;
	      lcapFace.ya = vertices[V_TOP].y;
	      lcapFace.k = -slopes[V_LEFT].k;
	    }
	  /* invoke miLineArc, isInt = false, to draw disk on end */
	  miLineArc (paintedSet, pixel, pGC,
		     &lcapFace, (LineFace *) NULL,
		     rcenterx, rcentery, false);
	}
    }

  /* work out left and right faces of the dashed segment, to pass back */
  leftFace->x = x1;
  leftFace->y = y1;
  leftFace->dx = dx;
  leftFace->dy = dy;
  leftFace->xa = rdy;
  leftFace->ya = -rdx;
  leftFace->k = k;

  rightFace->x = x2;
  rightFace->y = y2;
  rightFace->dx = -dx;
  rightFace->dy = -dy;
  rightFace->xa = -rdy;
  rightFace->ya = rdx;
  rightFace->k = k;

  /* update absolute dash number, dash index, dash offset */
  dashRemain = (int)(((double) dashRemain) - LRemain);
  if (dashRemain == 0)		/* on to next dash in array */
    {
      dashNum++;		/* bump absolute dash number */
      dashIndex++;
      if (dashIndex == pGC->numInDashList) /* wrap */
	dashIndex = 0;
      dashRemain = (int)(pDash[dashIndex]);
    }

  *pDashNum = dashNum;
  *pDashIndex = dashIndex;
  *pDashOffset = (int)(pDash[dashIndex]) - dashRemain;
}

/* Helper function, called by miWideDash() above and also by miZeroPolyArc
   (in mi_zerarc.c) and miZeroDash (in mi_zerolin.c) to perform initial
   offsetting into the dash array, before dash #0 is drawn.  In all cases,
   dashNum=0, dashIndex=0 and dashOffset=0. */

/* FIXME: a negative offset is not supported, and if it is, then some
   places elsewhere in the code, which assume dashNum>=0, may need to be
   fixed too. */
void
#ifdef _HAVE_PROTOS
miStepDash (int dist, int *pDashNum, int *pDashIndex, const unsigned int *pDash, int numInDashList, int *pDashOffset)
#else
miStepDash (dist, pDashNum, pDashIndex, pDash, numInDashList, pDashOffset)
     int dist;			/* additional offset (assumed >= 0) */
     int *pDashNum;		/* dash number */
     int *pDashIndex;		/* current dash */
     const unsigned int *pDash;	/* dash list */
     int numInDashList;		/* dashlist length */
     int *pDashOffset;		/* offset into current dash */
#endif
{
  int	dashNum, dashIndex, dashOffset;
  int   totallen;
  int	i;
    
  dashNum = *pDashNum;
  dashIndex = *pDashIndex;
  dashOffset = *pDashOffset;
  if (dashOffset + dist < (int)(pDash[dashIndex]))
    /* offset won't take us beyond end of present dash */
    {
      *pDashOffset = dashOffset + dist;
      return;
    }

  /* move to next dash */
  dist -= (int)(pDash[dashIndex]) - dashOffset;
  dashNum++;
  dashIndex++;
  if (dashIndex == numInDashList)
    /* wrap to beginning of dash list */
    dashIndex = 0;

  /* make it easy on ourselves: work modulo iteration interval */
  totallen = 0;
  for (i = 0; i < numInDashList; i++)
    totallen += (int)(pDash[i]);
  if (totallen <= dist)
    dist = dist % totallen;

  while (dist >= (int)(pDash[dashIndex]))
    {
      dist -= (int)(pDash[dashIndex]);
      dashNum++;
      dashIndex++;
      if (dashIndex == numInDashList)
	/* wrap to beginning of dash list */
	dashIndex = 0;
    }
  *pDashNum = dashNum;
  *pDashIndex = dashIndex;
  *pDashOffset = dist;
}


/* Draw a wide polyline, undashed (if width=0, this simply calls
   miZeroLine) */
void
#ifdef _HAVE_PROTOS
miWideLine (miPaintedSet *paintedSet, const miGC *pGC, miCoordMode mode, int npt, const miPoint *pPts)
#else
miWideLine (paintedSet, pGC, mode, npt, pPts)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     miCoordMode mode;
     int npt;
     const miPoint *pPts;
#endif
{
  int		    x1, y1, x2, y2;
  bool	            projectLeft, projectRight;
  LineFace	    leftFace, rightFace, prevRightFace;
  LineFace	    firstFace;
  int               first;
  bool	            somethingDrawn = false;
  bool	            selfJoin;

  /* ensure we have >=1 points */
  if (npt <= 0)
    return;

  /* width 0 lines are handled specially */
  if (pGC->lineWidth == 0)
    {
      miZeroLine (paintedSet, pGC, mode, npt, pPts);
      return;
    }

  x2 = pPts->x;
  y2 = pPts->y;
  first = true;

  /* determine whether polyline is closed */
  selfJoin = false;
  if (npt > 1)
    {
      if (mode == MI_COORD_MODE_PREVIOUS)
    	{
	  int nptTmp;
	  const miPoint *pPtsTmp;
    
	  x1 = x2;
	  y1 = y2;
	  nptTmp = npt;
	  pPtsTmp = pPts + 1;
	  while (--nptTmp)
	    {
	      x1 += pPtsTmp->x;
	      y1 += pPtsTmp->y;
	      ++pPtsTmp;
	    }
	  if (x2 == x1 && y2 == y1)
	    selfJoin = true;
    	}
      else if (x2 == pPts[npt-1].x && y2 == pPts[npt-1].y)
	selfJoin = true;
    }

  /* line segments (except for the last) will not project right; they'll
     project left if the cap mode is "projecting" */
  projectLeft = 
    (pGC->capStyle == (int)MI_CAP_PROJECTING && !selfJoin) ? true : false;
  projectRight = false;
  /* iterate through points, drawing all line segments of nonzero length */
  while (--npt)
    {
      x1 = x2;
      y1 = y2;
      ++pPts;
      x2 = pPts->x;
      y2 = pPts->y;
      if (mode == MI_COORD_MODE_PREVIOUS)
	{
	  x2 += x1;
	  y2 += y1;
	}
      if (x1 != x2 || y1 != y2)
	/* nonzero length */
	{
	  somethingDrawn = true;
	  if (npt == 1 && pGC->capStyle == (int)MI_CAP_PROJECTING && !selfJoin)
	    /* last point; and need a projecting cap here */
	    projectRight = true;
	  /* draw segment (pixel=1), returning faces */
	  miWideSegment (paintedSet, pGC->pixels[1], pGC, 
			 x1, y1, x2, y2,
			 projectLeft, projectRight, &leftFace, &rightFace);
	  if (first)
	    /* first line segment, draw round cap if needed */
	    {
	      if (selfJoin)
		firstFace = leftFace;
	      else if (pGC->capStyle == (int)MI_CAP_ROUND
		       || pGC->capStyle == (int)MI_CAP_TRIANGULAR)
		/* invoke miLineArc, isInt = true, to draw a round cap
		   on left face in paint type #1 */
		miLineArc (paintedSet, pGC->pixels[1], pGC,
			   &leftFace, (LineFace *)NULL,
			   (double)0.0, (double)0.0,
			   true);
	    }
	  else
	    /* general case: draw join at beginning of segment (pixel=1) */
	    miLineJoin (paintedSet, pGC->pixels[1], pGC,
			&leftFace, &prevRightFace);

	  prevRightFace = rightFace;
	  first = false;
	  projectLeft = false;
	}

      /* final point of polyline */
      if (npt == 1 && somethingDrawn)
 	{
	  if (selfJoin)
	    /* add line join to close the polyline, pixel=1 */
	    miLineJoin (paintedSet, pGC->pixels[1], pGC, 
			&firstFace, &rightFace);
	  else if (pGC->capStyle == (int)MI_CAP_ROUND
		   || pGC->capStyle == (int)MI_CAP_TRIANGULAR)
	    /* invoke miLineArc, isInt = true, to draw round cap
	       on right face, pixel=1 */
	    miLineArc (paintedSet, pGC->pixels[1], pGC,
		       (LineFace *)NULL, &rightFace,
		       (double)0.0, (double)0.0,
		       true);
	}
    }

  /* handle crock where all points are coincident */
  if (!somethingDrawn)
    {
      projectLeft = (pGC->capStyle == (int)MI_CAP_PROJECTING) ? true : false;
      miWideSegment (paintedSet, pGC->pixels[1], pGC, /* pixel=1 */
		     x2, y2, x2, y2, projectLeft, projectLeft,
		     &leftFace, &rightFace);
      if (pGC->capStyle == (int)MI_CAP_ROUND
	  || pGC->capStyle == (int)MI_CAP_TRIANGULAR)
	{
	  /* invoke miLineArc, isInt = true, to draw round cap
	     in paint type #1 */
	  miLineArc (paintedSet, pGC->pixels[1], pGC,
		     &leftFace, (LineFace *)NULL,
		     (double)0.0, (double)0.0,
		     true);
	  /* invoke miLineArc, isInt = true, to draw other round cap
	     in paint type #1 */
	  rightFace.dx = -1;	/* sleazy hack to make it work */
	  miLineArc (paintedSet, pGC->pixels[1], pGC,
		     (LineFace *) NULL, &rightFace,
		     (double)0.0, (double)0.0,
		     true);
	}
    }
}

/* Helper function, called by miWideLine() with pixel=1.  Draw a single
   segment of a wide polyline, taking into account projecting caps, but not
   round caps.  Also pass back left and right faces for the line segment,
   for possible use in adding caps or joins. */
static void
#ifdef _HAVE_PROTOS
miWideSegment (miPaintedSet *paintedSet, miPixel pixel, const miGC *pGC, int x1, int y1, int x2, int y2, bool projectLeft, bool projectRight, LineFace *leftFace, LineFace *rightFace)
#else
miWideSegment (paintedSet, pixel, pGC, x1, y1, x2, y2, projectLeft, projectRight, leftFace, rightFace)
     miPaintedSet *paintedSet;
     miPixel pixel;
     const miGC *pGC;
     int x1, y1, x2, y2;
     bool projectLeft, projectRight;
     LineFace *leftFace, *rightFace;
#endif
{
  double	l, L, r;
  double	xa, ya;
  double	projectXoff = 0.0, projectYoff = 0.0;
  double	k;
  double	maxy;
  int		dx, dy;
  int		x, y;
  int		finaly;
  PolyEdge      *left, *right;
  PolyEdge      *top, *bottom;
  int		lefty, righty, topy, bottomy;
  int		signdx;
  PolyEdge	lefts[2], rights[2];
  int		lw = (int)(pGC->lineWidth);

  if (y2 < y1 || (y2 == y1 && x2 < x1))
  /* interchange, so as always to draw top-to-bottom, or left-to-right if
     horizontal */
    {
      int tx, ty;
      bool tbool;
      LineFace *tface;

      tx = x1;
      x1 = x2;
      x2 = tx;

      ty = y1;
      y1 = y2;
      y2 = ty;

      tbool = projectLeft;
      projectLeft = projectRight;
      projectRight = tbool;

      tface = leftFace;
      leftFace = rightFace;
      rightFace = tface;
    }

  dy = y2 - y1;
  signdx = 1;
  dx = x2 - x1;
  if (dx < 0)
    signdx = -1;

  leftFace->x = x1;
  leftFace->y = y1;
  leftFace->dx = dx;
  leftFace->dy = dy;

  rightFace->x = x2;
  rightFace->y = y2;
  rightFace->dx = -dx;		/* for faces, (dx,dy) points _into_ line */
  rightFace->dy = -dy;

  if (dy == 0)
    /* segment is horizontal */
    {
      rightFace->xa = 0;
      rightFace->ya = 0.5 * (double)lw;
      rightFace->k = -0.5 * (double)(lw * dx); /* k = xa * dy - ya * dx */
      leftFace->xa = 0;
      leftFace->ya = -rightFace->ya;
      leftFace->k = rightFace->k; /* k = xa * dy - ya * dx */
      x = x1;
      if (projectLeft)
	x -= (lw >> 1);
      y = y1 - (lw >> 1);
      dx = x2 - x;
      if (projectRight)
	dx += ((lw + 1) >> 1);
      dy = lw;
      miFillRectPolyHelper (paintedSet, pixel, 
			    x, y, (unsigned int)dx, (unsigned int)dy);
    }
  else if (dx == 0)
    /* segment is vertical */
    {
      leftFace->xa =  0.5 * (double)lw;
      leftFace->ya = 0;
      leftFace->k = 0.5 * (double)(lw * dy); /* k = xa * dy - ya * dx */
      rightFace->xa = -leftFace->xa;
      rightFace->ya = 0;
      rightFace->k = leftFace->k; /* k = xa * dy - ya * dx */
      y = y1;
      if (projectLeft)
	y -= lw >> 1;
      x = x1 - (lw >> 1);
      dy = y2 - y;
      if (projectRight)
	dy += ((lw + 1) >> 1);
      dx = lw;
      miFillRectPolyHelper (paintedSet, pixel, 
			    x, y, (unsigned int)dx, (unsigned int)dy);
    }
  else
    /* general case: segment is neither horizontal nor vertical */
    {
      l = 0.5 * ((double) lw);
      L = hypot ((double) dx, (double) dy);

      if (dx < 0)
	{
	  right = &rights[1];
	  left = &lefts[0];
	  top = &rights[0];
	  bottom = &lefts[1];
	}
      else
	{
	  right = &rights[0];
	  left = &lefts[1];
	  top = &lefts[0];
	  bottom = &rights[1];
	}
      r = l / L;		/* this is ell / L, not 1 / L */

      ya = -r * dx;
      xa = r * dy;

      if (projectLeft | projectRight)
	{
	  projectXoff = -ya;
	  projectYoff = xa;
	}

      /* build first long edge */

      k = l * L;		/* xa * dy - ya * dx */
      leftFace->xa = xa;
      leftFace->ya = ya;
      leftFace->k = k;
      rightFace->xa = -xa;
      rightFace->ya = -ya;
      rightFace->k = k;

      if (projectLeft)
	righty = miPolyBuildEdge (xa - projectXoff, ya - projectYoff,
				  k, dx, dy,
				  x1, y1, false, right);
      else
	righty = miPolyBuildEdge (xa, ya,
				  k, dx, dy,
				  x1, y1, false, right);

      /* build second long edge */

      ya = -ya;
      xa = -xa;
      k = -k;			/* xa * dy - ya * dx */

      if (projectLeft)
	lefty = miPolyBuildEdge (xa - projectXoff, ya - projectYoff,
				 k, dx, dy, 
				 x1, y1, true, left);
      else
	lefty = miPolyBuildEdge (xa, ya,
				 k, dx, dy, 
				 x1, y1, true, left);

      /* build first short edge, on left end */

      if (signdx > 0)
	{
	  ya = -ya;
	  xa = -xa;
	}

      if (projectLeft)
	{
	  double xap = xa - projectXoff;
	  double yap = ya - projectYoff;
	  topy = miPolyBuildEdge (xap, yap, 
				  xap * dx + yap * dy, -dy, dx,
				  x1, y1, (dx > 0 ? true : false), top);
	}
      else
	topy = miPolyBuildEdge (xa, ya, 
				0.0, -dy, dx,
				x1, y1, (dx > 0 ? true : false), top);

      /* build second short edge, on right end */

      if (projectRight)
	{
	  double xap = xa + projectXoff;
	  double yap = ya + projectYoff;
	  bottomy = miPolyBuildEdge (xap, yap,
				     xap * dx + yap * dy, -dy, dx,
				     x2, y2, (dx < 0 ? true : false), bottom);
	  maxy = -ya + projectYoff;
	}
      else
	{
	  bottomy = miPolyBuildEdge (xa, ya,
				     0.0, -dy, dx,
				     x2, y2, (dx < 0 ? true : false), bottom);
	  maxy = -ya;
	}

      finaly = ICEIL (maxy) + y2;

      if (dx < 0)
	{
	  left->height = (unsigned int)(bottomy - lefty);
	  right->height = (unsigned int)(finaly - righty);
	  top->height = (unsigned int)(righty - topy);
	}
      else
	{
	  right->height = (unsigned int)(bottomy - righty);
	  left->height = (unsigned int)(finaly - lefty);
	  top->height = (unsigned int)(lefty - topy);
	}
      bottom->height = (unsigned int)(finaly - bottomy);

      /* fill the rectangle (2 left edges, 2 right edges) */
      miFillPolyHelper (paintedSet, pixel, topy,
			(unsigned int)(bottom->height + bottomy - topy),
			lefts, rights, 2, 2);
    }
}
