/* Original author:  Bob Scheifler, MIT X Consortium, mid 1980s.
   Hacked by Robert S. Maier <rsm@math.arizona.edu>, 1998-99 */

/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

/* This module contains the miZeroPolyArc() function and its reentrant
   counterpart miZeroPolyArc_r.  They draw single-pixel (Bresenham)
   polyarcs, either solid or dashed.  A fast integer algorithm is used.

   A polyarc is a list of arcs, which may or may not be contiguous.  An
   `arc' is an elliptic arc, i.e., a segment of an ellipse.  The principal
   axes of the ellipse must be aligned with the coordinate axes.
   
   The cap mode and join mode in the graphics context are ignored.

   All painting goes through the low-level MI_COPY_AND_PAINT_SPANS() and
   MI_PAINT_SPANS() macros. */

#include "sys-defines.h"
#include "extern.h"

#include "xmi.h"
#include "mi_spans.h"
#include "mi_gc.h"
#include "mi_api.h"
#include "mi_arc.h"
#include "mi_zerarc.h"

#define FULLCIRCLE (360 * 64)
#define OCTANT (45 * 64)
#define QUADRANT (90 * 64)
#define HALFCIRCLE (180 * 64)
#define QUADRANT3 (270 * 64)

/* trig functions, angle specified in 1/64 degrees */
#define Dsin(d)	((d) == 0 ? 0.0 : ((d) == QUADRANT ? 1.0 : \
				   ((d) == HALFCIRCLE ? 0.0 : \
				    ((d) == QUADRANT3 ? -1.0 : sin((double)d*(M_PI/11520.0))))))

#define Dcos(d)	((d) == 0 ? 1.0 : ((d) == QUADRANT ? 0.0 : \
				   ((d) == HALFCIRCLE ? -1.0 : \
				    ((d) == QUADRANT3 ? 0.0 : cos((double)d*(M_PI/11520.0))))))

#define EPSILON45 64

typedef struct 
{
  bool skipStart;
  bool haveStart;
  miPoint startPt;
  bool haveLast;
  bool skipLast;
  miPoint endPt;
  int dashNum;
  int dashIndex;
  int dashOffset;
  int dashNumInit;
  int dashIndexInit;
  int dashOffsetInit;
} miDashInfo;

static const miZeroArcPt _oob_arc_pt = {INT_MAX, INT_MAX, 0};

/* forward references */
static bool miZeroArcSetup ____P((const miArc *arc, miZeroArc *info, bool ok360));
static miPoint * miZeroArcPts ____P((const miArc *arc, miPoint *pts));
static void miZeroArcDashPts ____P((const miGC *pGC, const miArc *arc, miDashInfo *dinfo, int maxPts, miPoint **pts));


/*
 * This is the reentrant version, miZeroPolyArc_r.  The non-reentrant
 * version, miZeroPolyArc, simply calls this version, using an in-library
 * `rasterized ellipse' cache. */

void
#ifdef _HAVE_PROTOS
miZeroPolyArc_r (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs, miEllipseCache *ellipseCache)
#else
miZeroPolyArc_r (paintedSet, pGC, narcs, parcs, ellipseCache)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
     miEllipseCache *ellipseCache;
#endif
{
  const miArc *arc;
  miDashInfo dinfo;
  int j;

  if (pGC->lineStyle != (int)MI_LINE_SOLID)
    /* initialize structure used in dashing */
    {
      dinfo.haveStart = false;
      dinfo.skipStart = false;
      dinfo.haveLast = false;
      dinfo.dashIndexInit = 0;
      dinfo.dashNumInit = 0;
      dinfo.dashOffsetInit = 0;
      /* perform initial offsetting into the dash array */
      miStepDash (pGC->dashOffset, &dinfo.dashNumInit, &dinfo.dashIndexInit,
		  pGC->dash, pGC->numInDashList, &dinfo.dashOffsetInit);
    }

  for (arc = parcs, j = narcs; --j >= 0; arc++)
    {
      if (!MI_CAN_ZERO_ARC(arc))
	/* Too large an arc for integer algorithm to perform properly, so
	   hand it off to floating-point wide polyarc algorithm, which can
	   do zero-width polyarcs too. */
	/* N.B. This handoff is lame.  If dashing, dash pattern won't be
           carried over from arc to contiguous arc.  */
	miPolyArc_r (paintedSet, pGC, 1, arc, ellipseCache);

      else
	/* not unusually large, use integer Bresenham algorithm */
	{
	  miPoint **ptsInit, **pts;
	  int maxPts = 0, numPts, i, n;
	  int numPixels = pGC->numPixels;

	  if (arc->width > arc->height)
	    maxPts = arc->width + (arc->height >> 1);
	  else
	    maxPts = arc->height + (arc->width >> 1);
	  if (maxPts == 0)
	    continue;

	  /* max points produced by Bresenham algorithm (overestimate?) */
	  numPts = 4 * maxPts;

	  /* generate points (note that if dashing, dash pattern will carry
	     over from arc to contiguous arc) */
	  ptsInit = 
	    (miPoint **)mi_xmalloc(numPixels * sizeof(miPoint *));
	  pts = 
	    (miPoint **)mi_xmalloc(numPixels * sizeof(miPoint *));
	  if (pGC->lineStyle == (int)MI_LINE_SOLID)
	    {
	      for (i = 0; i < numPixels; i++)
		{
		  if (i == 1)
		    ptsInit[i] = (miPoint *)mi_xmalloc(numPts * sizeof(miPoint));
		  else		/* `solid' uses paint type #1 only */
		    ptsInit[i] = (miPoint *)NULL;
		  pts[i] = ptsInit[i];
		}
	      /* compute points, return pointer to slot after
                 last-generated point */
	      pts[1] = miZeroArcPts (arc, ptsInit[1]);
	    }
	  else			/* on/off dashed or double-dashed */
	    {
	      for (i = 0; i < numPixels; i++)
		{
		  ptsInit[i] = (miPoint *)mi_xmalloc(numPts * sizeof(miPoint));
		  pts[i] = ptsInit[i];
		}

	      /* compute points, return ptrs to ones after last-generated */
	      dinfo.skipLast = (i == 0 ? false : true);
	      miZeroArcDashPts (pGC, arc, &dinfo, maxPts, pts);
	      dinfo.skipStart = true;
	    }
	  
	  /* paint all generated points (except if not double-dashing,
	     don't paint points in paint type #0) */
	  for (i = 0; i < numPixels; i++)
	    {
	      if (ptsInit[i] == (miPoint *)NULL)
		continue;
	      if (i == 0 && pGC->lineStyle != (int)MI_LINE_DOUBLE_DASH)
		{
		  free (ptsInit[i]);
		  continue;
		}

	      n = pts[i] - ptsInit[i];
	      if (n > 0)
		{
		  unsigned int *widths;
		  int k;
	      
		  widths = (unsigned int *)mi_xmalloc(n * sizeof(unsigned int));
		  for (k = 0; k < n; k++)
		    widths[k] = 1;
		  miQuickSortSpansY (ptsInit[i], widths, n);
		  MI_PAINT_SPANS(paintedSet, pGC->pixels[i], n, ptsInit[i], widths)
	       }
	    } /* end of drawing loop over paint types */

	  /* free arrays of pointers to storage */
	  free (pts);
	  free (ptsInit);

	} /* end of integer Bresenham algorithm applied to a single arc */
    } /* end of loop over arcs */
}

#ifndef NO_NONREENTRANT_POLYARC_SUPPORT
void
#ifdef _HAVE_PROTOS
miZeroPolyArc (miPaintedSet *paintedSet, const miGC *pGC, int narcs, const miArc *parcs)
#else
miZeroPolyArc (paintedSet, pGC, narcs, parcs)
     miPaintedSet *paintedSet;
     const miGC *pGC;
     int narcs;
     const miArc *parcs;
#endif
{
  if (_mi_ellipseCache == (miEllipseCache *)NULL)
    _mi_ellipseCache = miNewEllipseCache ();
  miZeroPolyArc_r (paintedSet, pGC, narcs, parcs, _mi_ellipseCache);
}
#endif /* not NO_NONREENTRANT_POLYARC_SUPPORT */


#define Pixelate(pts, xval, yval) \
{ \
    pts->x = xval; \
    pts->y = yval; \
    pts++; \
}

#define DoPix(pts, mask, idx, xval, yval) \
	if (mask & (1 << idx)) Pixelate(pts, xval, yval);

/* Generate points that make up a solid zero-width arc, and write them to
   pre-allocated storage; return pointer to miPoint after last-generated
   point.  Storage supplied by caller, should be sufficiently large. */
static miPoint *
#ifdef _HAVE_PROTOS
miZeroArcPts (const miArc *arc, miPoint *pts)
#else
miZeroArcPts (arc, pts)
     const miArc *arc;
     miPoint *pts;
#endif
{
  miZeroArc info;
  int x, y, a, b, d;
  unsigned int mask;
  int k1, k3, dx, dy;
  bool do360;

  do360 = miZeroArcSetup(arc, &info, true);
  MIARCSETUP(info, x, y, k1, k3, a, b, d, dx, dy);
  mask = info.initialMask;
  if (!(arc->width & 1))	/* even width */
    {
      DoPix (pts, mask, 1, info.xorgo, info.yorg);
      DoPix (pts, mask, 3, info.xorgo, info.yorgo);
    }
  if (!info.end.x || !info.end.y)
    {
      mask = info.end.mask;
      info.end = info.altend;
    }
  if (do360 && (arc->width == arc->height) && !(arc->width & 1))
    /* full circle, even diameter */
    {
      int yorgh = info.yorg + info.h;
      int xorghp = info.xorg + info.h;
      int xorghn = info.xorg - info.h;

      for ( ; ; )
	{
	  Pixelate(pts, info.xorg + x, info.yorg + y);
	  Pixelate(pts, info.xorg - x, info.yorg + y);
	  Pixelate(pts, info.xorg - x, info.yorgo - y);
	  Pixelate(pts, info.xorg + x, info.yorgo - y);
	  if (a < 0)
	    break;
	  Pixelate(pts, xorghp - y, yorgh - x);
	  Pixelate(pts, xorghn + y, yorgh - x);
	  Pixelate(pts, xorghn + y, yorgh + x);
	  Pixelate(pts, xorghp - y, yorgh + x);
	  MIARCCIRCLESTEP(x, y, a, b, d, k1, k3, ;);
	}
      if (x > 1 && pts[-1].x == pts[-5].x && pts[-1].y == pts[-5].y)
	pts -= 4;
      x = info.w;
      y = info.h;
    }
  else if (do360)
    /* full ellipse */
    {
      while (y < (int)info.h || x < (int)info.w)
	{
	  MIARCOCTANTSHIFT(info, x, y, dx, dy, a, b, d, k1, k3, ;);
	  Pixelate(pts, info.xorg + x, info.yorg + y);
	  Pixelate(pts, info.xorgo - x, info.yorg + y);
	  Pixelate(pts, info.xorgo - x, info.yorgo - y);
	  Pixelate(pts, info.xorg + x, info.yorgo - y);
	  MIARCSTEP(x, y, dx, dy, a, b, d, k1, k3, ;, ;);
	}
    }
  else
    /* hard case */
    {
      while (y < (int)info.h || x < (int)info.w)
	{
	  MIARCOCTANTSHIFT(info, x, y, dx, dy, a, b, d, k1, k3, ;);
	  if ((x == info.start.x) || (y == info.start.y))
	    {
	      mask = info.start.mask;
	      info.start = info.altstart;
	    }
	  DoPix (pts, mask, 0, info.xorg + x, info.yorg + y);
	  DoPix (pts, mask, 1, info.xorgo - x, info.yorg + y);
	  DoPix (pts, mask, 2, info.xorgo - x, info.yorgo - y);
	  DoPix (pts, mask, 3, info.xorg + x, info.yorgo - y);
	  if ((x == info.end.x) || (y == info.end.y))
	    {
	      mask = info.end.mask;
	      info.end = info.altend;
	    }
	  MIARCSTEP(x, y, dx, dy, a, b, d, k1, k3, ;, ;);
	}
    }
  if ((x == info.start.x) || (y == info.start.y))
    mask = info.start.mask;
  DoPix (pts, mask, 0, info.xorg + x, info.yorg + y);
  DoPix (pts, mask, 2, info.xorgo - x, info.yorgo - y);
  if (arc->height & 1)		/* odd height */
    {
      DoPix (pts, mask, 1, info.xorgo - x, info.yorg + y);
      DoPix (pts, mask, 3, info.xorg + x, info.yorgo - y);
    }

  return pts;
}


#undef DoPix
#define DoPix(arcPts, mask, idx, xval, yval) \
if (mask & (1 << idx)) \
{ \
    arcPts[idx]->x = xval; \
    arcPts[idx]->y = yval; \
    arcPts[idx]++; \
}

/* Generate the points that make up a dashed zero-width arc, possibly
   multicolored; write them to pre-allocated storage, indexed by paint
   type. */
static void
#ifdef _HAVE_PROTOS
miZeroArcDashPts (const miGC *pGC, const miArc *arc, miDashInfo *dinfo, int maxPts, miPoint **pts)
#else
miZeroArcDashPts (pGC, arc, dinfo, maxPts, pts)
     const miGC *pGC;
     const miArc *arc;
     miDashInfo *dinfo;		/* updated vars (e.g. dashNum, dashIndex) */
     int maxPts;
     miPoint **pts;		/* pts[i] is storage for paint type #i */
#endif
{
  miZeroArc info;
  int x, y, a, b, d;
  unsigned int mask;
  int k1, k3, dx, dy;
  int dashRemaining, numPixels;
  miPoint *points, *arcPts[4];
  miPoint *startPts[5], *endPts[5];
  int deltas[5];
  miPoint *pt, *startPt, *lastPt;
  int i, j, seg, startseg;

  /* allocate temp storage, split into four pieces */
  points = (miPoint *)mi_xmalloc(sizeof(miPoint) * 4 * maxPts);
  for (i = 0; i < 4; i++)
    arcPts[i] = points + (i * maxPts);

  miZeroArcSetup (arc, &info, false);
  MIARCSETUP(info, x, y, k1, k3, a, b, d, dx, dy);
  mask = info.initialMask;
  startseg = info.startAngle / QUADRANT;
  startPt = arcPts[startseg];
  if (!(arc->width & 1))
    {
      DoPix (arcPts, mask, 1, info.xorgo, info.yorg);
      DoPix (arcPts, mask, 3, info.xorgo, info.yorgo);
    }
  if (!info.end.x || !info.end.y)
    {
      mask = info.end.mask;
      info.end = info.altend;
    }
  while (y < (int)info.h || x < (int)info.w)
    {
      MIARCOCTANTSHIFT(info, x, y, dx, dy, a, b, d, k1, k3, ;);
      if ((x == info.firstx) || (y == info.firsty))
	startPt = arcPts[startseg];
      if ((x == info.start.x) || (y == info.start.y))
	{
	  mask = info.start.mask;
	  info.start = info.altstart;
	}
      DoPix (arcPts, mask, 0, info.xorg + x, info.yorg + y);
      DoPix (arcPts, mask, 1, info.xorgo - x, info.yorg + y);
      DoPix (arcPts, mask, 2, info.xorgo - x, info.yorgo - y);
      DoPix (arcPts, mask, 3, info.xorg + x, info.yorgo - y);
      if ((x == info.end.x) || (y == info.end.y))
	{
	  mask = info.end.mask;
	  info.end = info.altend;
	}
      MIARCSTEP(x, y, dx, dy, a, b, d, k1, k3, ;, ;);
    }
  if ((x == info.firstx) || (y == info.firsty))
    startPt = arcPts[startseg];
  if ((x == info.start.x) || (y == info.start.y))
    mask = info.start.mask;
  DoPix (arcPts, mask, 0, info.xorg + x, info.yorg + y);
  DoPix (arcPts, mask, 2, info.xorgo - x, info.yorgo - y);
  if (arc->height & 1)
    {
      DoPix (arcPts, mask, 1, info.xorgo - x, info.yorg + y);
      DoPix (arcPts, mask, 3, info.xorg + x, info.yorgo - y);
    }
  for (i = 0; i < 4; i++)
    {
      seg = (startseg + i) & 3;
      pt = points + (seg * maxPts);
      if (seg & 1)
	{
	  startPts[i] = pt;
	  endPts[i] = arcPts[seg];
	  deltas[i] = 1;
	}
      else
	{
	  startPts[i] = arcPts[seg] - 1;
	  endPts[i] = pt - 1;
	  deltas[i] = -1;
	}
    }
  startPts[4] = startPts[0];
  endPts[4] = startPt;
  startPts[0] = startPt;
  if (startseg & 1)
    {
      if (startPts[4] != endPts[4])
	endPts[4]--;
      deltas[4] = 1;
    }
  else
    {
      if (startPts[0] > startPts[4])
	startPts[0]--;
      if (startPts[4] < endPts[4])
	endPts[4]--;
      deltas[4] = -1;
    }
  if (arc->angle2 < 0)
    {
      miPoint *tmps, *tmpe;
      int tmpd;

      tmpd = deltas[0];
      tmps = startPts[0] - tmpd;
      tmpe = endPts[0] - tmpd;
      startPts[0] = endPts[4] - deltas[4];
      endPts[0] = startPts[4] - deltas[4];
      deltas[0] = -deltas[4];
      startPts[4] = tmpe;
      endPts[4] = tmps;
      deltas[4] = -tmpd;
      tmpd = deltas[1];
      tmps = startPts[1] - tmpd;
      tmpe = endPts[1] - tmpd;
      startPts[1] = endPts[3] - deltas[3];
      endPts[1] = startPts[3] - deltas[3];
      deltas[1] = -deltas[3];
      startPts[3] = tmpe;
      endPts[3] = tmps;
      deltas[3] = -tmpd;
      tmps = startPts[2] - deltas[2];
      startPts[2] = endPts[2] - deltas[2];
      endPts[2] = tmps;
      deltas[2] = -deltas[2];
    }
  for (i = 0; i < 5 && startPts[i] == endPts[i]; i++)
    ;
  if (i == 5)
    return;
  pt = startPts[i];
  for (j = 4; startPts[j] == endPts[j]; j--)
    ;
  lastPt = endPts[j] - deltas[j];
  if (dinfo->haveLast &&
      (pt->x == dinfo->endPt.x) && (pt->y == dinfo->endPt.y))
    startPts[i] += deltas[i];
  else				/* not contiguous; restart dash pattern */
    {
      dinfo->dashNum = dinfo->dashNumInit;
      dinfo->dashIndex = dinfo->dashIndexInit;
      dinfo->dashOffset = dinfo->dashOffsetInit;
    }
  if (!dinfo->skipStart && (info.startAngle != info.endAngle))
    {
      dinfo->startPt = *pt;
      dinfo->haveStart = true;
    }
  else if (!dinfo->skipLast && dinfo->haveStart &&
	   (lastPt->x == dinfo->startPt.x) &&
	   (lastPt->y == dinfo->startPt.y) &&
	   (lastPt != startPts[i]))
    endPts[j] = lastPt;
  if (info.startAngle != info.endAngle)
    {
      dinfo->haveLast = true;
      dinfo->endPt = *lastPt;
    }

  /* iterate through generated points, updating dash information (e.g.,
     dashNum and paint type), writing points in paint type `i' into
     pre-allocated array pts[i] */

  dashRemaining = (int)pGC->dash[dinfo->dashIndex] - dinfo->dashOffset;
  numPixels = pGC->numPixels;
  for (i = 0; i < 5; i++)
    {
      int delta;

      pt = startPts[i];
      lastPt = endPts[i];
      delta = deltas[i];
      while (pt != lastPt)
	{
	  int dashNum, paintType;

	  /* use a paint type that cycles through 1..(numPixels-1) for
	     even-numbered dashes, and is 0 for odd-numbered ones */
	  dashNum = dinfo->dashNum;
	  paintType = (dashNum & 1) ? 0 : 1 + ((dashNum / 2) % (numPixels-1));
	  while ((pt != lastPt) && --dashRemaining >= 0)
	    {
	      *(pts[paintType]++) = *pt;
	      pt += delta;
	    }

	  if (dashRemaining <= 0)
	    /* on to next dash */
	    {
	      ++(dinfo->dashNum);
	      if (++(dinfo->dashIndex) == pGC->numInDashList)
		/* loop to beginning of dash array */
		dinfo->dashIndex = 0;
	      dashRemaining = (int)pGC->dash[dinfo->dashIndex];
	    }
	}
    }

  /* pass back amount left in now-current dash, so that dash pattern will
     continue from arc to contiguous arc */
  dinfo->dashOffset = (int)pGC->dash[dinfo->dashIndex] - dashRemaining;

  /* free temp storage */
  free (points);
}


/*
 * (x - l)^2 / (W/2)^2  + (y + H/2)^2 / (H/2)^2 = 1
 *
 * where l is either 0 or .5
 *
 * alpha = 4(W^2)
 * beta = 4(H^2)
 * gamma = 0
 * u = 2(W^2)H
 * v = 4(H^2)l
 * k = -4(H^2)(l^2)
 *
 */

/* Helper function called by ZeroArcPts() and ZeroArcDashPts() above.
   Generates a miZeroArc struct for any specified arc. */

static bool
#ifdef _HAVE_PROTOS
miZeroArcSetup (const miArc *arc, miZeroArc *info, bool ok360)
#else
miZeroArcSetup (arc, info, ok360)
     const miArc *arc;
     miZeroArc *info;
     bool ok360;
#endif
{
  int l, i;
  int angle1, angle2;
  int startseg, endseg;
  int startAngle, endAngle;
  miZeroArcPt start, end;
  bool overlap;

  l = arc->width & 1;
  if (arc->width == arc->height) /* circular arc */
    {
      info->alpha = 4;
      info->beta = 4;
      info->k1 = -8;
      info->k3 = -16;
      info->b = 12;
      info->a = (arc->width << 2) - 12;
      info->d = 17 - (arc->width << 1);
      if (l)
	{
	  info->b -= 4;
	  info->a += 4;
	  info->d -= 7;
	}
    }
  else if (arc->width == 0 || arc->height == 0)	/* degenerate arc */
    {
      info->alpha = 0;
      info->beta = 0;
      info->k1 = 0;
      info->k3 = 0;
      info->a = -(int)arc->height;
      info->b = 0;
      info->d = -1;
    }
  else				/* non-degenerate non-circular arc */
    {
      /* initial conditions */
      info->alpha = (arc->width * arc->width) << 2;
      info->beta = (arc->height * arc->height) << 2;
      info->k1 = info->beta << 1;
      info->k3 = info->k1 + (info->alpha << 1);
      info->b = l ? 0 : -info->beta;
      info->a = info->alpha * arc->height;
      info->d = info->b - (info->a >> 1) - (info->alpha >> 2);
      if (l)
	info->d -= info->beta >> 2;
      info->a -= info->b;
      /* take first step, d < 0 always */
      info->b -= info->k1;
      info->a += info->k1;
      info->d += info->b;
      /* octant change, b < 0 always */
      info->k1 = -info->k1;
      info->k3 = -info->k3;
      info->b = -info->b;
      info->d = info->b - info->a - info->d;
      info->a = info->a - (info->b << 1);
    }

  info->dx = 1;
  info->dy = 0;
  info->w = (arc->width + 1) >> 1;
  info->h = arc->height >> 1;
  info->xorg = arc->x + (arc->width >> 1);
  info->yorg = arc->y;
  info->xorgo = info->xorg + l;
  info->yorgo = info->yorg + arc->height;
  if (arc->width == 0)
    {
      if (arc->height == 0)
	{
	  info->x = 0;
	  info->y = 0;
	  info->initialMask = 0;
	  info->startAngle = 0;
	  info->endAngle = 0;
	  info->start = _oob_arc_pt;
	  info->end = _oob_arc_pt;
	  return false;
	}
      info->x = 0;
      info->y = 1;
    }
  else
    {
      info->x = 1;
      info->y = 0;
    }

  angle1 = arc->angle1;
  angle2 = arc->angle2;
  if ((angle1 == 0) && (angle2 >= FULLCIRCLE))
    {
      startAngle = 0;
      endAngle = 0;
    }
  else
    {
      if (angle2 > FULLCIRCLE)
	angle2 = FULLCIRCLE;
      else if (angle2 < -FULLCIRCLE)
	angle2 = -FULLCIRCLE;
      if (angle2 < 0)
	{
	  startAngle = angle1 + angle2;
	  endAngle = angle1;
	}
      else
	{
	  startAngle = angle1;
	  endAngle = angle1 + angle2;
	}
      if (startAngle < 0)
	startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
      if (startAngle >= FULLCIRCLE)
	startAngle = startAngle % FULLCIRCLE;
      if (endAngle < 0)
	endAngle = FULLCIRCLE - (-endAngle) % FULLCIRCLE;
      if (endAngle >= FULLCIRCLE)
	endAngle = endAngle % FULLCIRCLE;
    }

  info->startAngle = startAngle;
  info->endAngle = endAngle;
  if (ok360 && (startAngle == endAngle) && arc->angle2 
      && arc->width && arc->height)
    {
      info->initialMask = 0xf;
      info->start = _oob_arc_pt;
      info->end = _oob_arc_pt;
      return true;
    }
  startseg = startAngle / OCTANT;
  if (!arc->height || (((startseg + 1) & 2) && arc->width))
    {
      start.x = (int)(Dcos(startAngle) * ((arc->width + 1) / 2.0));
      if (start.x < 0)
	start.x = -start.x;
      start.y = -1;
    }
  else
    {
      start.y = (int)(Dsin(startAngle) * (arc->height / 2.0));
      if (start.y < 0)
	start.y = -start.y;
      start.y = info->h - start.y;
      start.x = INT_MAX;
    }
  endseg = endAngle / OCTANT;
  if (!arc->height || (((endseg + 1) & 2) && arc->width))
    {
      end.x = (int)(Dcos(endAngle) * ((arc->width + 1) / 2.0));
      if (end.x < 0)
	end.x = -end.x;
      end.y = -1;
    }
  else
    {
      end.y = (int)(Dsin(endAngle) * (arc->height / 2.0));
      if (end.y < 0)
	end.y = -end.y;
      end.y = info->h - end.y;
      end.x = INT_MAX;
    }
  info->firstx = start.x;
  info->firsty = start.y;
  info->initialMask = 0;
  overlap = ((arc->angle2 != 0) && (endAngle <= startAngle)) ? true : false;
  for (i = 0; i < 4; i++)
    {
      if (overlap ?
	  ((i * QUADRANT <= endAngle) || ((i + 1) * QUADRANT > startAngle)) :
	  ((i * QUADRANT <= endAngle) && ((i + 1) * QUADRANT > startAngle)))
	info->initialMask |= (1 << i);
    }
  start.mask = info->initialMask;
  end.mask = info->initialMask;
  startseg >>= 1;
  endseg >>= 1;
  overlap = (overlap && (endseg == startseg)) ? true : false;
  if (start.x != end.x || start.y != end.y || !overlap)
    {
      if (startseg & 1)
	{
	  if (!overlap)
	    info->initialMask &= ~(1 << startseg);
	  if (start.x > end.x || start.y > end.y)
	    end.mask &= ~(1 << startseg);
	}
      else
	{
	  start.mask &= ~(1 << startseg);
	  if (((start.x < end.x || start.y < end.y) ||
	       (start.x == end.x && start.y == end.y && (endseg & 1))) &&
	      !overlap)
	    end.mask &= ~(1 << startseg);
	}
      if (endseg & 1)
	{
	  end.mask &= ~(1 << endseg);
	  if (((start.x > end.x || start.y > end.y) ||
	       (start.x == end.x && start.y == end.y && !(startseg & 1))) &&
	      !overlap)
	    start.mask &= ~(1 << endseg);
	}
      else
	{
	  if (!overlap)
	    info->initialMask &= ~(1 << endseg);
	  if (start.x < end.x || start.y < end.y)
	    start.mask &= ~(1 << endseg);
	}
    }
  /* take care of case when start and stop are both near 45 */
  /* handle here rather than adding extra code to pixelization loops */
  if (startAngle &&
      ((start.y < 0 && end.y >= 0) || (start.y >= 0 && end.y < 0)))
    {
      i = (startAngle + OCTANT) % OCTANT;
      if (i < EPSILON45 || i > OCTANT - EPSILON45)
	{
	  i = (endAngle + OCTANT) % OCTANT;
	  if (i < EPSILON45 || i > OCTANT - EPSILON45)
	    {
	      if (start.y < 0)
		{
		  i = (int)(Dsin(startAngle) * (arc->height / 2.0));
		  if (i < 0)
		    i = -i;
		  if ((int)info->h - i == end.y)
		    start.mask = end.mask;
		}
	      else
		{
		  i = (int)(Dsin(endAngle) * (arc->height / 2.0));
		  if (i < 0)
		    i = -i;
		  if ((int)info->h - i == start.y)
		    end.mask = start.mask;
		}
	    }
	}
    }
  if (startseg & 1)
    {
      info->start = start;
      info->end = _oob_arc_pt;
    }
  else
    {
      info->end = start;
      info->start = _oob_arc_pt;
    }
  if (endseg & 1)
    {
      info->altend = end;
      if (info->altend.x < info->end.x || info->altend.y < info->end.y)
	{
	  miZeroArcPt tmp;
	  tmp = info->altend;
	  info->altend = info->end;
	  info->end = tmp;
	}
      info->altstart = _oob_arc_pt;
    }
  else
    {
      info->altstart = end;
      if (info->altstart.x < info->start.x ||
	  info->altstart.y < info->start.y)
	{
	  miZeroArcPt tmp;
	  tmp = info->altstart;
	  info->altstart = info->start;
	  info->start = tmp;
	}
      info->altend = _oob_arc_pt;
    }
  if (!info->start.x || !info->start.y)
    {
      info->initialMask = info->start.mask;
      info->start = info->altstart;
    }
  if (!arc->width && (arc->height == 1))
    {
      /* kludge! */
      info->initialMask |= info->end.mask;
      info->initialMask |= info->initialMask << 1;
      info->end.x = 0;
      info->end.mask = 0;
    }
  return false;
}
